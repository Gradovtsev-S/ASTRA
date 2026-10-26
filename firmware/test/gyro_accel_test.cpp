#include <Arduino.h>
#include <Wire.h>
#include <math.h>

// =====================================================================
// ЧАСТЬ 1. ЗАВИСИТ ОТ ЧИПА (BMI160)
// =====================================================================
static const uint8_t IMU_ADDR        = 0x69;      // SAO подтянут к 3V3
static const float   ACC_LSB_PER_G   = 16384.0f;  // диапазон ±2 g
static const float   GYR_LSB_PER_DPS = 65.6f;     // диапазон ±500 °/с

static void writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(IMU_ADDR);
    Wire.write(reg);
    Wire.write(val);
    Wire.endTransmission();
}

static int readReg(uint8_t reg) {
    Wire.beginTransmission(IMU_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return -1;
    if (Wire.requestFrom(IMU_ADDR, (uint8_t)1) != 1) return -1;
    return Wire.read();
}

static bool imu_chip_init() {
    if (readReg(0x00) != 0xD1) return false;   // CHIP_ID BMI160

    writeReg(0x7E, 0xB6);   // программный сброс, чтобы начать с известного состояния
    delay(100);
    writeReg(0x7E, 0x11);   // акселерометр: нормальный режим
    delay(5);
    writeReg(0x7E, 0x15);   // гироскоп: нормальный режим
    delay(85);
    writeReg(0x41, 0x03);   // акселерометр ±2 g
    writeReg(0x43, 0x02);   // гироскоп ±500 °/с
    delay(10);

    return readReg(0x41) == 0x03 && readReg(0x43) == 0x02;   // запись дошла
}

// Читает гироскоп (°/с) и акселерометр (g) одним запросом.
static bool imu_chip_read(float &gx, float &gy, float &gz,
                          float &ax, float &ay, float &az) {
    Wire.beginTransmission(IMU_ADDR);
    Wire.write(0x0C);                                   // GYR_X_L: тут начинаются данные
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(IMU_ADDR, (uint8_t)12) != 12) return false;

    uint8_t b[12];
    for (auto &v : b) v = Wire.read();

    // младший байт первым
    auto s16 = [&](int i) { return (int16_t)((b[i + 1] << 8) | b[i]); };

    gx = s16(0)  / GYR_LSB_PER_DPS;
    gy = s16(2)  / GYR_LSB_PER_DPS;
    gz = s16(4)  / GYR_LSB_PER_DPS;
    ax = s16(6)  / ACC_LSB_PER_G;
    ay = s16(8)  / ACC_LSB_PER_G;
    az = s16(10) / ACC_LSB_PER_G;
    // Если оси стоят «не так» (знак), правим ЗДЕСЬ, например ax = -ax;
    return true;
}

// =====================================================================
// ЧАСТЬ 2. НЕ ЗАВИСИТ ОТ ЧИПА
// =====================================================================
static void tilt_from_accel(float ax, float ay, float az,
                            float &roll, float &pitch) {
    const float RAD2DEG = 57.2958f;
    roll  = atan2f(ay, az) * RAD2DEG;
    pitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD2DEG;
}

static float gbx = 0, gby = 0, gbz = 0;   // смещение нуля гироскопа, °/с

// Усредняет показания гироскопа в покое. Плата должна быть неподвижна.
static bool gyro_calibrate(int samples) {
    float sx = 0, sy = 0, sz = 0;
    int ok = 0;
    for (int i = 0; i < samples; i++) {
        float gx, gy, gz, ax, ay, az;
        if (imu_chip_read(gx, gy, gz, ax, ay, az)) {
            sx += gx; sy += gy; sz += gz;
            ok++;
        }
        delay(10);
    }
    if (ok < samples / 2) return false;   // слишком много ошибок чтения
    gbx = sx / ok; gby = sy / ok; gbz = sz / ok;
    return true;
}

// =====================================================================
// ЧАСТЬ 3. ТЕСТ
// =====================================================================
void setup() {
    Serial.begin(115200);
    delay(1000);
    Wire.begin(21, 22);

    if (!imu_chip_init()) {
        Serial.println("BMI160: ОШИБКА инициализации");
        return;
    }
    Serial.println("BMI160: OK");
    Serial.printf("PMU_STATUS = 0x%02X (ожидаем 0x14: оба датчика в нормальном режиме)\n",
                  readReg(0x03));

    Serial.println("Не двигайте плату, калибровка гироскопа 2 с...");
    if (gyro_calibrate(200))
        Serial.printf("Смещение нуля гироскопа: %.2f %.2f %.2f °/с\n", gbx, gby, gbz);
    else
        Serial.println("Калибровка гироскопа: ОШИБКА чтения");
}

void loop() {
    static uint32_t lastUs = micros();
    static uint32_t lastPrint = 0;
    static float yaw = 0;   // угол вокруг оси Z интегрированием

    float gx, gy, gz, ax, ay, az;
    if (!imu_chip_read(gx, gy, gz, ax, ay, az)) {
        Serial.println("Ошибка чтения BMI160");
        delay(100);
        return;
    }

    gx -= gbx; gy -= gby; gz -= gbz;   // вычитаем смещение нуля

    uint32_t now = micros();
    float dt = (now - lastUs) / 1e6f;  // секунды с прошлого замера
    lastUs = now;
    yaw += gz * dt;                    // интегрирование: угол += скорость * время

    if (millis() - lastPrint >= 200) {
        lastPrint = millis();
        float roll, pitch;
        tilt_from_accel(ax, ay, az, roll, pitch);
        Serial.printf("A[g] %6.3f %6.3f %6.3f | крен %6.1f тангаж %6.1f | "
                      "G[°/с] %6.2f %6.2f %6.2f | yaw %7.1f\n",
                      ax, ay, az, roll, pitch, gx, gy, gz, yaw);
    }
    delay(10);   // ~100 Гц, как частота датчика
}