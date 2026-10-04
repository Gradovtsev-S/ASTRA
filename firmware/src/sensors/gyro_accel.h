#pragma once
#include <stdbool.h>

// Наклон неподвижного основания, градусы.
// Знаки: roll > 0 при наклоне вправо, pitch > 0 при наклоне вперёд (уточни под монтаж BMI160).
struct GyroAccelData {
    float roll;    // -180..180
    float pitch;   //  -90..90
    bool  valid;
};

bool gyro_accel_init(void);

// Калибровка нуля, платформа неподвижна. Блокирующая.
void gyro_accel_calibrate(void);

// Опрос чипа, вызывается из задачи датчиков
void gyro_accel_update(void);

// Потокобезопасно
GyroAccelData gyro_accel_get_data(void);