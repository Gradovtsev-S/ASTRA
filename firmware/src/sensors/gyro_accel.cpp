
#include "gyro_accel.h"
#include "sensor_chips.h"
#include "../config.h"
#include <Arduino.h>

namespace {

portMUX_TYPE lock = portMUX_INITIALIZER_UNLOCKED;
GyroAccelData data{};
bool ready = false;

// Калибровка ADXL345 по шести положениям.
// Смещение задаётся в g, масштаб — безразмерный.
constexpr SensorVector accelBias{
    -0.0395f,
     0.0040f,
    -0.0623f
};

constexpr SensorVector accelScale{
    1.0199f,
    1.0363f,
    1.0447f
};

// Применяем калибровку к сырым данным в осях чипа.
SensorVector calibrateAcceleration(const SensorVector& raw) {
    return {
        (raw.x - accelBias.x) * accelScale.x,
        (raw.y - accelBias.y) * accelScale.y,
        (raw.z - accelBias.z) * accelScale.z
    };
}

} // namespace

bool gyro_accel_init() {
    ready = adxl345_init();
    return ready;
}

void gyro_accel_calibrate() {
    // Статическая калибровка применяется в gyro_accel_update().
    // Повторная калибровка во время работы пока не реализована.
}

void gyro_accel_update() {
    GyroAccelData next{};

    SensorVector raw{};
    SensorVector mounted{};

    if (ready && adxl345_read(raw)) {
        const SensorVector corrected = calibrateAcceleration(raw);

        next.valid =
            orientation_map(corrected, ADXL_MOUNT_AXES, mounted) &&
            orientation_tilt(mounted, next.roll, next.pitch);
    }

    if (next.valid) {
        next.roll -= IMU_ROLL_OFFSET_DEG;
        next.pitch -= IMU_PITCH_OFFSET_DEG;
    }

    portENTER_CRITICAL(&lock);
    data = next;
    portEXIT_CRITICAL(&lock);
}

GyroAccelData gyro_accel_get_data() {
    portENTER_CRITICAL(&lock);
    const GyroAccelData result = data;
    portEXIT_CRITICAL(&lock);

    return result;
}
