#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// =====================================================================
// ЧАСТЬ 1. ЗАВИСИТ ОТ ЧИПА (сейчас GY-511 / LSM303DLHC)
// При замене на BMI160 переписывается только этот блок.
// =====================================================================
static const uint8_t ACC_ADDR      = 0x19;
static const float   ACC_LSB_PER_G = 1000.0f;   // после сдвига >> 4

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

// Включает датчик. true, если он ответил и настройка применилась.
static bool acc_chip_init() {
    writeReg(ACC_ADDR, 0x20, 0x57);   // 100 Гц, оси X, Y, Z
    writeReg(ACC_ADDR, 0x23, 0x88);   // ±2 g, высокая точность, BDU
    delay(20);
    return readReg(ACC_ADDR, 0x20) == 0x57;   // проверка: запись дошла
}

// Читает ускорения в единицах g. true, если чтение удалось.
static bool acc_chip_read_g(float &ax, float &ay, float &az) {
    Wire.beginTransmission(ACC_ADDR);
    Wire.write(0x28 | 0x80);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(ACC_ADDR, (uint8_t)6) != 6) return false;

    uint8_t b[6];
    for (auto &v : b) v = Wire.read();

    int16_t x = (int16_t)((b[1] << 8) | b[0]) >> 4;
    int16_t y = (int16_t)((b[3] << 8) | b[2]) >> 4;
    int16_t z = (int16_t)((b[5] << 8) | b[4]) >> 4;

    ax = x / ACC_LSB_PER_G;
    ay = y / ACC_LSB_PER_G;
    az = z / ACC_LSB_PER_G;
    // Если оси стоят «не так» (знак или перестановка), правим ЗДЕСЬ:
    // например, ax = -ax;
    return true;
}

// =====================================================================
// ЧАСТЬ 2. НЕ ЗАВИСИТ ОТ ЧИПА
// Работает с ускорениями в g. Менять при замене датчика не нужно.
// =====================================================================
static void tilt_from_accel(float ax, float ay, float az,
                            float &roll, float &pitch) {
    const float RAD2DEG = 57.2958f;
    roll  = atan2f(ay, az) * RAD2DEG;
    pitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD2DEG;
}

// =====================================================================
// ЧАСТЬ 3. ТЕСТ
// =====================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);
    Wire.begin(21, 22);
    Serial.println(acc_chip_init() ? "Акселерометр: OK" : "Акселерометр: ОШИБКА");
}

void loop() {
    float ax, ay, az;
    if (acc_chip_read_g(ax, ay, az)) {
        float roll, pitch;
        tilt_from_accel(ax, ay, az, roll, pitch);
        Serial.printf("X=%6.3f Y=%6.3f Z=%6.3f g | крен=%6.1f тангаж=%6.1f\n",
                      ax, ay, az, roll, pitch);
    } else {
        Serial.println("Ошибка чтения акселерометра");
    }
    delay(500);
}