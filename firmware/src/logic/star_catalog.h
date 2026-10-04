#pragma once
#include <stdbool.h>

// Поиск звезды по имени без учёта регистра. Координаты J2000:
// ra_h в часах (0..24), dec_deg в градусах.
// Планеты, Солнце и Луну каталог не содержит, они считаются в astronomy.
bool star_catalog_find(const char* name, double* ra_h, double* dec_deg);