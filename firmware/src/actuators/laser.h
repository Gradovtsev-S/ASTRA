#pragma once
#include <stdbool.h>

// Пин на выход, после инициализации лазер выключен
bool laser_init(void);

// Автоотключение по таймеру LASER_MAX_ON_MS (config.h) реализуется внутри.
// Условия включения (цель достигнута, status == Ok) проверяет вызывающий код.
void laser_on(void);
void laser_off(void);
bool laser_status(void);