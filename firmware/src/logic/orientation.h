#pragma once
#include "handler.h"

struct SensorVector { float x, y, z; };

// Правосторонняя система основания: X вперёд (az=0), Y вправо, Z вниз.
// Отсчёты осей: ±1 = ±X, ±2 = ±Y, ±3 = ±Z. Нули означают неизвестный монтаж.
struct SensorAxes { int x, y, z; };

bool orientation_map(SensorVector raw, SensorAxes axes, SensorVector& mounted);
bool orientation_tilt(SensorVector up, float& roll, float& pitch);
bool orientation_heading(SensorVector field, float roll, float pitch,
                         float declination, float& magnetic, float& heading);
TargetAngles orientation_to_mount(double az, double elevation, const Orientation& orient);
