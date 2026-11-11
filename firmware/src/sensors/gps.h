#pragma once

#include <stdint.h>
#include <stdbool.h>

struct GpsData {
    double lat;
    double lon;
    double elev;

    uint16_t year;
    uint8_t month, day;
    uint8_t hour, min, sec;

    bool pos_valid;
    bool time_valid;
};

// Инициализация GPS-модуля.
bool gps_init(void);

// Чтение UART и снимок данных. Вызывать из одного цикла опроса.
GpsData gps_get_data(void);

// Ручная установка координат, приоритет над GPS до перезапуска.
void gps_set_manual(double lat, double lon, double elev);