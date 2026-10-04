#pragma once
#include <stdbool.h>

struct MagCompassData {
    float magnetic;    // 0..360°, магнитный азимут нуля монтировки (то, что видит датчик)
    float azimuth;     // 0..360°, истинный азимут = magnetic + склонение.
                       // Это значение использует handler.
    bool  calibrated;
    bool  valid;
};

// Инициализация по I2C, загрузка калибровки из NVS.
// Склонение при старте берётся из MAG_DECLINATION_DEG (config.h).
bool mag_compass_init(void);

// Калибровка вращением, блокирующая. Результат сохраняется в NVS.
void mag_compass_calibrate(void);

// Магнитное склонение, градусы, + к востоку (Долгопрудный ~ +12°).
// Нужно только если хочешь поменять значение из config.h на лету.
void mag_compass_set_declination(float deg);

// Опрос чипа. roll/pitch из gyro_accel, градусы (компенсация наклона).
void mag_compass_update(float roll, float pitch);

// Потокобезопасно
MagCompassData mag_compass_get_data(void);