
#include "gps.h"
#include "../config.h"
#include "../logic/handler.h"

#include <Arduino.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

// UART для GPS.
static HardwareSerial gpsSerial(GPS_UART_NUM);
static bool ready = false;
static uint32_t positionAt = 0;
static uint32_t timeAt = 0;
static constexpr uint32_t kMaxAgeMs = 3000;
static GpsData manualPosition{};

// Последние известные данные.
static GpsData gpsData = {};

// Буфер для NMEA-предложения.
static char nmeaBuffer[128];
static size_t nmeaIndex = 0;

// Защита общих данных между задачами FreeRTOS.
static portMUX_TYPE gpsMux = portMUX_INITIALIZER_UNLOCKED;


// ------------------------------------------------------------
// Преобразование координат NMEA в десятичные градусы.
// Например: 5545.1234 -> 55.752056...
// ------------------------------------------------------------
static double number(const char* value) {
    char* end;
    const double result = strtod(value, &end);
    return end != value && *end == '\0' && isfinite(result) ? result : NAN;
}

static double nmeaToDegrees(const char* value, const char* hemisphere,
                            char positive, char negative, double limit) {
    if ((hemisphere[0] != positive && hemisphere[0] != negative) || hemisphere[1])
        return NAN;
    const double raw = number(value);
    if (!isfinite(raw) || raw < 0 || raw > 18000)
        return NAN;
    const double degrees = floor(raw / 100);
    const double minutes = raw - degrees * 100;
    const double result = degrees + minutes / 60;
    if (minutes >= 60 || result > limit)
        return NAN;
    return hemisphere[0] == negative ? -result : result;
}

static bool digits(const char* value, size_t count) {
    for (size_t i = 0; i < count; ++i)
        if (value[i] < '0' || value[i] > '9')
            return false;
    return true;
}

// ------------------------------------------------------------
// Проверка контрольной суммы NMEA.
// Формат: $GNRMC,...*HH
// ------------------------------------------------------------
static bool checkChecksum(const char* sentence) {
    if (sentence == nullptr || sentence[0] != '$') {
        return false;
    }

    const char* star = strchr(sentence, '*');

    if (star == nullptr || strlen(star) != 3 ||
        !isxdigit(static_cast<unsigned char>(star[1])) ||
        !isxdigit(static_cast<unsigned char>(star[2]))) {
        return false;
    }

    uint8_t checksum = 0;

    for (const char* p = sentence + 1; p < star; ++p) {
        checksum ^= static_cast<uint8_t>(*p);
    }

    char expected[3] = {
        star[1],
        star[2],
        '\0'
    };

    char* end = nullptr;
    unsigned long received = strtoul(expected, &end, 16);

    if (end == expected || *end != '\0') {
        return false;
    }

    return checksum == received;
}


// ------------------------------------------------------------
// Разделение NMEA-предложения на поля.
// Пустые поля сохраняются.
// ------------------------------------------------------------
static int splitFields(char* sentence, char* fields[], int maxFields) {
    int count = 0;

    if (sentence == nullptr || maxFields <= 0) {
        return 0;
    }

    fields[count++] = sentence;

    for (char* p = sentence; *p != '\0'; ++p) {
        if (*p == ',' || *p == '*') {
            *p = '\0';

            if (count < maxFields) {
                fields[count++] = p + 1;
            }
        }
    }

    return count;
}


// ------------------------------------------------------------
// Разбор времени UTC из RMC.
// Формат: HHMMSS.sss
// ------------------------------------------------------------
static bool parseTime(const char* value, GpsData& data) {
    if (strlen(value) < 6 || !digits(value, 6)) {
        return false;
    }

    if (value[6] && (value[6] != '.' || !value[7] || !digits(value + 7, strlen(value + 7))))
        return false;

    char hour[3] = { value[0], value[1], '\0' };
    char minute[3] = { value[2], value[3], '\0' };
    char second[3] = { value[4], value[5], '\0' };

    int h = atoi(hour);
    int m = atoi(minute);
    int s = atoi(second);

    if (h > 23 || m > 59 || s > 59) {
        return false;
    }

    data.hour = static_cast<uint8_t>(h);
    data.min = static_cast<uint8_t>(m);
    data.sec = static_cast<uint8_t>(s);

    return true;
}


// ------------------------------------------------------------
// Разбор даты из RMC.
// Формат: DDMMYY
// ------------------------------------------------------------
static bool parseDate(const char* value, GpsData& data) {
    if (strlen(value) != 6 || !digits(value, 6)) {
        return false;
    }

    char day[3] = { value[0], value[1], '\0' };
    char month[3] = { value[2], value[3], '\0' };
    char year[3] = { value[4], value[5], '\0' };

    int d = atoi(day);
    int m = atoi(month);
    int y = atoi(year);

    if (d < 1 || d > 31 || m < 1 || m > 12) {
        return false;
    }

    data.day = static_cast<uint8_t>(d);
    data.month = static_cast<uint8_t>(m);

    // Для GPS-дат 00-79 считаем годами 2000-2079.
    data.year = static_cast<uint16_t>(
        y < 80 ? 2000 + y : 1900 + y
    );

    return true;
}


// ------------------------------------------------------------
// Обработка сообщения RMC.
// Содержит время, дату и координаты.
// ------------------------------------------------------------
static void parseRMC(char* fields[], int count) {
    // Минимум: тип, время, статус, широта, N/S,
    // долгота, E/W, скорость, курс, дата.
    if (count < 10) {
        gpsData.time_valid = false;
        gpsData.pos_valid = false;
        return;
    }

    GpsData updated = gpsData;
    updated.time_valid = parseTime(fields[1], updated) && parseDate(fields[9], updated) &&
        utc_valid({updated.year, updated.month, updated.day, updated.hour, updated.min,
                   static_cast<double>(updated.sec)});
    if (updated.time_valid)
        timeAt = millis();

    // RMC сообщает потерю фиксации. Высота и позиция приходят вместе из GGA.
    if (strcmp(fields[2], "A") != 0)
        updated.pos_valid = false;
    gpsData = updated;
}

static void parseGGA(char* fields[], int count) {
    gpsData.pos_valid = false;
    if (count < 11 || strcmp(fields[10], "M") != 0)
        return;

    const double quality = number(fields[6]);
    const double lat = nmeaToDegrees(fields[2], fields[3], 'N', 'S', 90);
    const double lon = nmeaToDegrees(fields[4], fields[5], 'E', 'W', 180);
    const double elev = number(fields[9]);
    if (!isfinite(quality) || quality < 1 || quality > 5 || floor(quality) != quality ||
        !isfinite(lat) || !isfinite(lon) || !isfinite(elev))
        return;

    gpsData.lat = lat;
    gpsData.lon = lon;
    gpsData.elev = elev;
    gpsData.pos_valid = true;
    positionAt = millis();
}

// Обработка полного NMEA-предложения
static void processNMEA(char* sentence) {
    if (!checkChecksum(sentence)) {
        return;
    }

    // Удаляем '$'.
    char* content = sentence + 1;

    char* fields[20];
    int count = splitFields(content, fields, 20);

    if (count == 0 || strlen(fields[0]) != 5) {
        return;
    }

    // Поддерживаем GP, GN и другие префиксы talker ID.
    if (strcmp(fields[0] + 2, "RMC") == 0) {
        parseRMC(fields, count);
    }
    else if (strcmp(fields[0] + 2, "GGA") == 0) {
        parseGGA(fields, count);
    }
}


// Чтение UART и сборка NMEA-предложений
static void readGPS() {
    for (int remaining = 512; remaining > 0 && gpsSerial.available(); --remaining) {
        char c = static_cast<char>(gpsSerial.read());

        if (c == '$') {
            // Начинается новое предложение.
            nmeaIndex = 0;
            nmeaBuffer[nmeaIndex++] = c;
        }
        else if (c == '\n') {
            if (nmeaIndex > 0) {
                nmeaBuffer[nmeaIndex] = '\0';
                processNMEA(nmeaBuffer);
                nmeaIndex = 0;
            }
        }
        else if (nmeaIndex > 0 && nmeaIndex < sizeof(nmeaBuffer) - 1) {
            if (c != '\r') {
                nmeaBuffer[nmeaIndex++] = c;
            }
        }
        else if (nmeaIndex >= sizeof(nmeaBuffer) - 1) {
            // Слишком длинное предложение.
            nmeaIndex = 0;
        }
    }
}



// Инициализация GPS
bool gps_init(void) {
    gpsData = {};
    nmeaIndex = 0;
    ready = GPS_RX_PIN >= 0 && GPS_BAUD > 0;
    if (ready)
        gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    return ready;
}

GpsData gps_get_data() {
    if (ready)
        readGPS();
    GpsData result = gpsData;
    const uint32_t now = millis();
    gpsData.pos_valid = result.pos_valid = result.pos_valid && now - positionAt <= kMaxAgeMs;
    gpsData.time_valid = result.time_valid = result.time_valid && now - timeAt <= kMaxAgeMs;

    portENTER_CRITICAL(&gpsMux);
    if (manualPosition.pos_valid) {
        result.lat = manualPosition.lat;
        result.lon = manualPosition.lon;
        result.elev = manualPosition.elev;
        result.pos_valid = true;
    }
    portEXIT_CRITICAL(&gpsMux);
    return result;
}

void gps_set_manual(double lat, double lon, double elev) {
    if (!isfinite(lat) || fabs(lat) > 90 || !isfinite(lon) || fabs(lon) > 180 ||
        !isfinite(elev))
        return;

    portENTER_CRITICAL(&gpsMux);
    manualPosition.lat = lat;
    manualPosition.lon = lon;
    manualPosition.elev = elev;
    manualPosition.pos_valid = true;
    portEXIT_CRITICAL(&gpsMux);
}
