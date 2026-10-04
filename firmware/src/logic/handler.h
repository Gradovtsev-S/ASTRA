#pragma once
#include <stdint.h>

// Положение установки
struct Observer {
    double lat;     // широта, градусы (+ север)
    double lon;     // долгота, градусы (+ восток)
    double elev;    // высота над уровнем моря, м
};

// Время UTC
struct UtcTime {
    uint16_t year;
    uint8_t  month, day;
    uint8_t  hour, min;
    double   sec;
};

// Ориентация основания
struct Orientation {
    float roll;     // крен, градусы
    float pitch;    // тангаж, градусы
    float heading;  // истинный азимут направления az = 0 монтировки,
                    // от севера по часовой, 0..360
};

enum class TargetStatus : uint8_t {
    Ok,
    UnknownObject,
};

// Углы в системе монтировки, готовые для motors_set_target
struct TargetAngles {
    double az;      // 0..360
    double alt;     // 0..180: 0 = лазер вниз, 90 = горизонт, 180 = зенит
    TargetStatus status;
};

TargetAngles handler_calculate(const char* target_name,
                               const Observer& obs,
                               const UtcTime& time,
                               const Orientation& orient);