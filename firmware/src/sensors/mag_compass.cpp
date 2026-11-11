
#include "mag_compass.h"
#include "sensor_chips.h"
#include "../config.h"

#include <Arduino.h>
#include <math.h>

namespace {

portMUX_TYPE lock = portMUX_INITIALIZER_UNLOCKED;

MagCompassData data{};

float declination = MAG_DECLINATION_DEG;
bool ready = false;

// Максимальный возраст последнего измерения, мс.
constexpr uint32_t MAX_SAMPLE_AGE_MS = 300;

// Предварительная XY-калибровка HMC5883L.
// Смещения получены при горизонтальном вращении.
// Единицы измерения: мкТл.
constexpr float MAG_BIAS_X = -14.66f;
constexpr float MAG_BIAS_Y =  16.22f;

// Последнее успешно считанное измерение.
SensorVector lastRaw{};
uint32_t lastSampleTime = 0;
bool hasSample = false;

// Коррекция магнитного поля в осях датчика.
SensorVector calibrateMagneticField(const SensorVector& raw) {
    return {
        raw.x - MAG_BIAS_X,
        raw.y - MAG_BIAS_Y,
        raw.z
    };
}

} // namespace

bool mag_compass_init() {
    ready = hmc5883l_init();

    hasSample = false;
    lastSampleTime = 0;
    lastRaw = {};

    portENTER_CRITICAL(&lock);
    data = {};
    portEXIT_CRITICAL(&lock);

    return ready;
}

void mag_compass_calibrate() {
    // Полная 3D-калибровка пока не реализована.
    // Предварительная XY-коррекция применяется автоматически.
}

void mag_compass_update(float roll, float pitch) {
    portENTER_CRITICAL(&lock);
    const float correction = declination;
    portEXIT_CRITICAL(&lock);

    MagCompassData next{};

    if (!ready) {
        portENTER_CRITICAL(&lock);
        data = next;
        portEXIT_CRITICAL(&lock);
        return;
    }

    SensorVector raw{};

    // Считываем новое измерение, если оно доступно.
    if (hmc5883l_read(raw)) {
        lastRaw = raw;
        lastSampleTime = millis();
        hasSample = true;
    }

    const uint32_t now = millis();

    // Используем измерение, только пока оно актуально.
    if (hasSample &&
        uint32_t(now - lastSampleTime) <= MAX_SAMPLE_AGE_MS) {

        // Применяем XY-калибровку в осях HMC5883L.
        const SensorVector calibrated =
            calibrateMagneticField(lastRaw);

        SensorVector mounted{};

        // Преобразуем в систему координат ASTRA.
        if (orientation_map(
                calibrated,
                HMC_MOUNT_AXES,
                mounted)) {

            // Рассчитываем направление с компенсацией наклона.
            next.valid = orientation_heading(
                mounted,
                roll,
                pitch,
                correction,
                next.magnetic,
                next.azimuth
            );
        }
    }

    portENTER_CRITICAL(&lock);
    data = next;
    portEXIT_CRITICAL(&lock);
}

MagCompassData mag_compass_get_data() {
    portENTER_CRITICAL(&lock);
    const MagCompassData result = data;
    portEXIT_CRITICAL(&lock);

    return result;
}

void mag_compass_set_declination(float deg) {
    if (!isfinite(deg))
        return;

    portENTER_CRITICAL(&lock);
    declination = deg;
    portEXIT_CRITICAL(&lock);
}

