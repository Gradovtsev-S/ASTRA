#pragma once
#include <stdbool.h>

// Углы в системе монтировки, градусы:
//   az:  0..360, 0 = направление, заданное при домировании, по часовой
//   alt: 0..360, 0 = лазер вниз, 90 = горизонт, 180 = зенит, 270 = горизонт
//        с обратной стороны, 360 = снова вниз

// Пины STEP/DIR/EN обоих A4988, генератор импульсов
bool motors_init(void);

// Запуск поиска дома по датчикам Холла в фоне, возврат сразу
void motors_home(void);
bool motors_is_homed(void);

// Новая цель (можно вызывать на лету при слежении). Азимут идёт кратчайшим путём.
// false, если не отдомились или углы вне диапазона.
bool motors_set_target(double az, double alt);

bool   motors_is_target_reached(void);
double motors_get_current_az(void);
double motors_get_current_alt(void);

// hold = true: вал удерживается, false: обмотки обесточены
void motors_stop(bool hold);