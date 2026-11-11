#include "sensor_chips.h"
#include "../config.h"
#include <Wire.h>

namespace {
constexpr uint8_t ADXL_ADDRESS = IMU_I2C_ADDR;
constexpr uint8_t HMC_ADDRESS = MAG_I2C_ADDR;

int readRegister(uint8_t address, uint8_t reg) {
    Wire.beginTransmission(address);
    Wire.write(reg);

    if (Wire.endTransmission(false) != 0)
        return -1;

    if (Wire.requestFrom(address, uint8_t(1)) != 1)
        return -1;

    return Wire.read();
}

bool writeRegister(
    uint8_t address,
    uint8_t reg,
    uint8_t value
) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(value);

    return Wire.endTransmission() == 0;
}

// Interpret a 16-bit two's-complement value.
int32_t signed16(uint16_t raw) {
    if (raw < 0x8000u)
        return static_cast<int32_t>(raw);

    return static_cast<int32_t>(raw) - 65536;
}

} // namespace

// ---------------- ADXL345 ----------------

bool adxl345_init() {
    if (readRegister(ADXL_ADDRESS, 0x00) != 0xE5)
        return false;

    // Standby before configuration.
    if (!writeRegister(ADXL_ADDRESS, 0x2D, 0x00))
        return false;

    // FULL_RES, +/-2g.
    if (!writeRegister(ADXL_ADDRESS, 0x31, 0x08))
        return false;

    // Measurement mode.
    return writeRegister(ADXL_ADDRESS, 0x2D, 0x08);
}

bool adxl345_read(SensorVector& acceleration) {
    const int source = readRegister(ADXL_ADDRESS, 0x30);
    if (source < 0 || (source & 0x80) == 0)
        return false;

    Wire.beginTransmission(ADXL_ADDRESS);
    Wire.write(0x32);

    if (Wire.endTransmission(false) != 0)
        return false;

    if (Wire.requestFrom(ADXL_ADDRESS, uint8_t(6)) != 6)
        return false;

    uint16_t raw[3];
    for (auto& axis : raw) {
        const uint8_t low = Wire.read();
        const uint8_t high = Wire.read();
        axis = low | (uint16_t(high) << 8);
    }

    constexpr float SCALE = 0.00390625f;

    acceleration.x = signed16(raw[0]) * SCALE;
    acceleration.y = signed16(raw[1]) * SCALE;
    acceleration.z = signed16(raw[2]) * SCALE;

    return true;
}

// ---------------- HMC5883L ----------------

bool hmc5883l_init() {
    // Identification: H43.
    if (readRegister(HMC_ADDRESS, 0x0A) != 0x48)
        return false;

    if (readRegister(HMC_ADDRESS, 0x0B) != 0x34)
        return false;

    if (readRegister(HMC_ADDRESS, 0x0C) != 0x33)
        return false;

    // 8 samples average, 15 Hz.
    if (!writeRegister(HMC_ADDRESS, 0x00, 0x70))
        return false;

    // +/-1.3 Gauss, 1090 LSB/Gauss.
    if (!writeRegister(HMC_ADDRESS, 0x01, 0x20))
        return false;

    // Continuous measurement mode.
    return writeRegister(HMC_ADDRESS, 0x02, 0x00);
}

bool hmc5883l_read(SensorVector& magnetic) {
    const int status = readRegister(HMC_ADDRESS, 0x09);

    if (status < 0 || (status & 0x03) == 0)
        return false;

    Wire.beginTransmission(HMC_ADDRESS);
    Wire.write(0x03);

    if (Wire.endTransmission(false) != 0)
        return false;

    if (Wire.requestFrom(HMC_ADDRESS, uint8_t(6)) != 6)
        return false;

    // HMC5883L register order: X, Z, Y.
    const uint8_t xHigh = Wire.read();
    const uint8_t xLow = Wire.read();

    const uint8_t zHigh = Wire.read();
    const uint8_t zLow = Wire.read();

    const uint8_t yHigh = Wire.read();
    const uint8_t yLow = Wire.read();

    const int32_t x = signed16(
        (uint16_t(xHigh) << 8) | xLow
    );

    const int32_t y = signed16(
        (uint16_t(yHigh) << 8) | yLow
    );

    const int32_t z = signed16(
        (uint16_t(zHigh) << 8) | zLow
    );

    // Полное чтение снимает LOCK после прерванной транзакции.
    if ((status & 0x02) || x == -4096 || y == -4096 || z == -4096)
        return false;

    constexpr float SCALE = 100.0f / 1090.0f;

    magnetic.x = x * SCALE;
    magnetic.y = y * SCALE;
    magnetic.z = z * SCALE;

    return true;
}
