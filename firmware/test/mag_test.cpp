#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// =====================================================================
// ЧАСТЬ 1. ЗАВИСИТ ОТ ЧИПА (магнитометр GY-511 / LSM303DLHC)
// =====================================================================
static const uint8_t MAG_ADDR            = 0x1E;
static const float   MAG_LSB_PER_GAUSS_XY = 1100.0f;   // при ±1.3 Гс
static const float   MAG_LSB_PER_GAUSS_Z  = 980.0f;

static void writeReg(uint8_t addr, uint8_t reg, uint8_t val) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

static int readReg(uint8_t addr, uint8_t reg) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return -1;
    if (Wire.requestFrom(addr, (uint8_t)1) != 1) return -1;
    return Wire.read();
}

static bool mag_chip_init() {
    // проверка «имени» чипа: H, 0x34, 0x33
    if (readReg(MAG_ADDR, 0x0A) != 0x48) return false;
    if (readReg(MAG_ADDR, 0x0B) != 0x34) return false;
    if (readReg(MAG_ADDR, 0x0C) != 0x33) return false;

    writeReg(MAG_ADDR, 0x00, 0x14);   // 30 Гц
    writeReg(MAG_ADDR, 0x01, 0x20);   // ±1.3 Гс
    writeReg(MAG_ADDR, 0x02, 0x00);   // непрерывный режим
    delay(20);
    return true;
}

// Читает сырые значения. Порядок регистров X, Z, Y; старший байт первым.
static bool mag_chip_read_raw(int16_t &x, int16_t &y, int16_t &z) {
    Wire.beginTransmission(MAG_ADDR);
    Wire.write(0x03);                                   // OUT_X_H_M
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(MAG_ADDR, (uint8_t)6) != 6) return false;

    uint8_t b[6];
    for (auto &v : b) v = Wire.read();

    x = (int16_t)((b[0] << 8) | b[1]);
    z = (int16_t)((b[2] << 8) | b[3]);
    y = (int16_t)((b[4] << 8) | b[5]);
    return true;
}

// =====================================================================
// ЧАСТЬ 3. ТЕСТ
// =====================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);
    Wire.begin(21, 22);
    Serial.println(mag_chip_init() ? "Магнитометр: OK" : "Магнитометр: ОШИБКА");
}

static const uint32_t CAL_TIME_MS = 20000;
static int16_t xmin = 32767, xmax = -32768;
static int16_t ymin = 32767, ymax = -32768;

void loop() {
    static bool     first = true;
    static uint32_t t0 = 0;
    static uint32_t lastPrint = 0;
    static bool     calDone = false;
    static float    cx = 0, cy = 0;
    static float    sx = 0, sy = 0;
    static int      n = 0;

    if (first) {
        first = false;
        t0 = millis();
        Serial.println("Калибровка 20 с: вращайте плату плашмя, 2-3 оборота");
    }

    int16_t x, y, z;
    if (!mag_chip_read_raw(x, y, z)) {
        Serial.println("Ошибка чтения магнитометра");
        delay(100);
        return;
    }

    // ---- Фаза 1: сбор экстремумов
    if (millis() - t0 < CAL_TIME_MS) {
        if (x < xmin) xmin = x;
        if (x > xmax) xmax = x;
        if (y < ymin) ymin = y;
        if (y > ymax) ymax = y;
        if (millis() - lastPrint > 1000) {
            lastPrint = millis();
            Serial.printf("  X: %d..%d   Y: %d..%d\n", xmin, xmax, ymin, ymax);
        }
        delay(30);
        return;
    }

    // ---- Переход: фиксируем центр
    if (!calDone) {
        cx = (xmax + xmin) / 2.0f;
        cy = (ymax + ymin) / 2.0f;
        calDone = true;
        Serial.printf("Готово. Центр=(%.0f; %.0f), размах X=%d, Y=%d\n",
                      cx, cy, xmax - xmin, ymax - ymin);
    }

    // ---- Фаза 2: курс, усреднение по 10 замерам
    sx += x - cx;
    sy += y - cy;
    n++;
    if (n < 10) { delay(40); return; }

    float heading = atan2f(sy / n, sx / n) * 57.2958f;
    if (heading < 0) heading += 360.0f;
    Serial.printf("курс=%6.1f\n", heading);
    sx = 0; sy = 0; n = 0;
    delay(40);
}