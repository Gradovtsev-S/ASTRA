#include "handler.h"
#include <math.h>
#include "astronomy.h" 

static const astro_refraction_t kRefraction = REFRACTION_NORMAL;
static const double kRollSign  = 1.0;
static const double kPitchSign = 1.0;
namespace {

struct NamedBody {
    const char*  name;
    astro_body_t body;
};

const NamedBody kBodies[] = {
    { "sun",     BODY_SUN     },
    { "moon",    BODY_MOON    },
    { "mercury", BODY_MERCURY },
    { "venus",   BODY_VENUS   },
    { "mars",    BODY_MARS    },
    { "jupiter", BODY_JUPITER },
    { "saturn",  BODY_SATURN  },
    { "uranus",  BODY_URANUS  },
    { "neptune", BODY_NEPTUNE },
    { "pluto",   BODY_PLUTO   },
};
bool equals_nocase(const char* s, const char* lower){
    for (; *s && *lower; ++s, ++lower) {
        char c = (*s >= 'A' && *s <= 'Z') ? (char)(*s - 'A' + 'a') : *s;
        if (c != *lower) return false;
    }
    return *s == *lower;
}
bool find_body(const char* name, astro_body_t* body){
    if (name == nullptr) return false;
    for (const NamedBody& b : kBodies){
        if (equals_nocase(name, b.name)){
            *body = b.body;
            return true;
        }
    }
    return false;
}
struct Vec3 { double x, y, z; };
Vec3 rot_z(const Vec3& v, double a){
    double c = cos(a), s = sin(a);
    return { c * v.x + s * v.y, -s * v.x + c * v.y, v.z };
}
Vec3 rot_y(const Vec3& v, double a){
    double c = cos(a), s = sin(a);
    return { c * v.x - s * v.z, v.y, s * v.x + c * v.z };
}
Vec3 rot_x(const Vec3& v, double a){
    double c = cos(a), s = sin(a);
    return { v.x, c * v.y + s * v.z, -s * v.y + c * v.z };
}
double wrap360(double deg){
    deg = fmod(deg, 360.0);
    return deg < 0.0 ? deg + 360.0 : deg;
}
double clamp_unit(double v){
    return v > 1.0 ? 1.0 : (v < -1.0 ? -1.0 : v);
}
} 
TargetAngles handler_calculate(const char* target_name,
                               const Observer& obs,
                               const UtcTime& time,
                               const Orientation& orient)
{
    TargetAngles out = { 0.0, 0.0, TargetStatus::UnknownObject };
    astro_body_t body;
    if (!find_body(target_name, &body))
        return out;

    astro_time_t     t   = Astronomy_MakeTime(time.year, time.month, time.day,
                                              time.hour, time.min, time.sec);
    astro_observer_t loc = Astronomy_MakeObserver(obs.lat, obs.lon, obs.elev);

    astro_equatorial_t eq = Astronomy_Equator(body, &t, loc, EQUATOR_OF_DATE, ABERRATION);
    if (eq.status != ASTRO_SUCCESS)
        return out;

    astro_horizon_t hor = Astronomy_Horizon(&t, loc, eq.ra, eq.dec, kRefraction);

    double az  = hor.azimuth  * DEG2RAD;
    double alt = hor.altitude * DEG2RAD;
    Vec3 v = { cos(alt) * cos(az), cos(alt) * sin(az), -sin(alt) };

    v = rot_z(v, (double)orient.heading * DEG2RAD);
    v = rot_y(v, kPitchSign * (double)orient.pitch * DEG2RAD);
    v = rot_x(v, kRollSign  * (double)orient.roll  * DEG2RAD);
    out.az     = wrap360(atan2(v.y, v.x) * RAD2DEG);
    out.alt    = acos(clamp_unit(v.z)) * RAD2DEG;
    out.status = TargetStatus::Ok;
    return out;
}