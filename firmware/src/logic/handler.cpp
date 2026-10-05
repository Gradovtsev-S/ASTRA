#include "handler.h"
#include "star_catalog.h"



// вкл астрономи энджн
extern "C" {
#include "astronomy.h"  
}

#include <math.h>
#include <string.h>

static const astro_refraction_t kRefraction = REFRACTION_NORMAL;

// Знаки углов IMU. Принято: крен > 0 — правая сторона основания вниз,
// тангаж > 0 — направление az = 0 поднято вверх (носом вверх).
// Если ваш IMU считает наоборот, поменяйте знак на -1.
static const double kRollSign  = 1.0;
static const double kPitchSign = 1.0;

// Расстояние для звёзд, св. лет. Нужно только чтобы задать звезду в Astronomy
// Engine; параллакс при таких расстояниях пренебрежимо мал.
static const double kStarDistanceLy = 1000.0;

static const double kDeg2Rad = M_PI / 180.0;
static const double kRad2Deg = 180.0 / M_PI;

// ---------------------------------------------------------------------------
// Вспомогательное
// ---------------------------------------------------------------------------

static char lower_ascii(char c)
{
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}

static bool equals_nocase(const char* a, const char* b)
{
    while (*a && *b) {
        if (lower_ascii(*a) != lower_ascii(*b)) return false;
        ++a; ++b;
    }
    return *a == *b;
}

struct BodyName{
    const char*  name;
    astro_body_t body;
};

static const BodyName kBodies[] = {
    { "Sun",     BODY_SUN     },
    { "Moon",    BODY_MOON    },
    { "Mercury", BODY_MERCURY },
    { "Venus",   BODY_VENUS   },
    { "Mars",    BODY_MARS    },
    { "Jupiter", BODY_JUPITER },
    { "Saturn",  BODY_SATURN  },
    { "Uranus",  BODY_URANUS  },
    { "Neptune", BODY_NEPTUNE },
    { "Pluto",   BODY_PLUTO   },
};

// Подбирает тело Astronomy Engine по имени: сначала Солнце/Луна/планеты,
// затем каталог звёзд (звезда задаётся в слоте BODY_STAR1).
static bool resolve_body(const char* name, astro_body_t* body)
{
    for (const BodyName& b : kBodies){
        if (equals_nocase(name, b.name)){
            *body = b.body;
            return true;
        }
    }

    double ra_h = 0.0, dec_deg = 0.0;
    if (!star_catalog_find(name, &ra_h, &dec_deg))
        return false;

    if (Astronomy_DefineStar(BODY_STAR1, ra_h, dec_deg, kStarDistanceLy) != ASTRO_SUCCESS)
        return false;

    *body = BODY_STAR1;
    return true;
}

// ---------------------------------------------------------------------------
// Преобразование «горизонт наблюдателя» -> «система монтировки»
//
// Локальная система NED: x = север, y = восток, z = вниз.
// Система основания: x = направление az = 0, y = вправо, z = вниз.
// Переход NED -> основание: v_b = Rx(roll) * Ry(pitch) * Rz(heading).
//
// Углы монтировки: az — от оси x по часовой (к y), alt — угол от оси «вниз»:
//   d = ( sin(alt)cos(az), sin(alt)sin(az), cos(alt) )
// alt = 0 — вниз, 90 — горизонт, 180 — зенит.
// ---------------------------------------------------------------------------

struct Vec3 { double x, y, z; };
static Vec3 rot_z(const Vec3& v, double a){
    const double c = cos(a), s = sin(a);
    return { c * v.x + s * v.y, -s * v.x + c * v.y, v.z };
}
static Vec3 rot_y(const Vec3& v, double a){
    const double c = cos(a), s = sin(a);
    return { c * v.x - s * v.z, v.y, s * v.x + c * v.z };
}
static Vec3 rot_x(const Vec3& v, double a){
    const double c = cos(a), s = sin(a);
    return { v.x, c * v.y + s * v.z, -s * v.y + c * v.z };
}

static double wrap360(double deg){
    deg = fmod(deg, 360.0);
    if (deg < 0.0) deg += 360.0;
    return deg;
}

static double clamp1(double v){
    return v > 1.0 ? 1.0 : (v < -1.0 ? -1.0 : v);
}







// Основные вычисления
TargetAngles handler_calculate(const char* target_name, const Observer& obs, const UtcTime& time, const Orientation& orient){
    TargetAngles out = { 0.0, 0.0, TargetStatus::UnknownObject };
    if (target_name == nullptr)
        return out;
    astro_body_t body;
    if (!resolve_body(target_name, &body))
        return out;


    astro_time_t t = Astronomy_MakeTime(time.year, time.month, time.day, time.hour, time.min, time.sec);
    astro_observer_t loc = Astronomy_MakeObserver(obs.lat, obs.lon, obs.elev);


    // Топоцентрические экваториальные координаты на дату (с аберрацией).
    astro_equatorial_t eq = Astronomy_Equator(body, &t, loc, EQUATOR_OF_DATE, ABERRATION);
    if (eq.status != ASTRO_SUCCESS)
        return out;


    // Азимут (от севера по часовой) и высота над горизонтом.
    astro_horizon_t hor = Astronomy_Horizon(&t, loc, eq.ra, eq.dec, kRefraction);
    // Единичный вектор на цель в NED.
    const double az_h  = hor.azimuth  * kDeg2Rad;
    const double alt_h = hor.altitude * kDeg2Rad;
    Vec3 v = { cos(alt_h) * cos(az_h), cos(alt_h) * sin(az_h), -sin(alt_h) };


    // NED -> система основания.
    v = rot_z(v,  (double)orient.heading * kDeg2Rad);
    v = rot_y(v,  kPitchSign * (double)orient.pitch * kDeg2Rad);
    v = rot_x(v,  kRollSign  * (double)orient.roll  * kDeg2Rad);

    
    // Вектор -> углы монтировки.
    out.az  = wrap360(atan2(v.y, v.x) * kRad2Deg);  
    out.alt = acos(clamp1(v.z)) * kRad2Deg;
    out.status = TargetStatus::Ok;
    return out;
}