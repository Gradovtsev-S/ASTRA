#pragma once
#include <stdbool.h>
#include "handler.h"

// Входные данные handler в одном пакете
struct HandlerInputs {
    Observer    obs;
    UtcTime     time;
    Orientation orient;
};

// Берёт данные датчиков (gps, gyro_accel, mag_compass) и переводит их в типы handler.
// false, если какие-то данные недостоверны: считать наведение нельзя.
bool adapter_read(HandlerInputs* out);
