#include "handler.h"
#include "orientation.h"
#include <math.h>
#include "astronomy.h"

static const astro_refraction_t kRefraction = REFRACTION_NORMAL;
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
    for (; *s && *lower; ++s, ++lower){
        char c = (*s >= 'A' && *s <= 'Z') ? (char)(*s - 'A' + 'a') : *s;
        if (c != *lower)
            return false;
    }
    return *s == *lower;
}
bool find_body(const char* name, astro_body_t* body){
    if (name == nullptr)
        return false;
    for (const NamedBody& b : kBodies){
        if (equals_nocase(name, b.name)){
            *body = b.body;
            return true;
        }
    }
    return false;
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

    if (!isfinite(obs.lat) || fabs(obs.lat) > 90 ||
        !isfinite(obs.lon) || fabs(obs.lon) > 180 || !isfinite(obs.elev) ||
        !utc_valid(time))
        return {0, 0, TargetStatus::InvalidInput};

    astro_time_t     t   = Astronomy_MakeTime(time.year, time.month, time.day,
                                              time.hour, time.min, time.sec);
    astro_observer_t loc = Astronomy_MakeObserver(obs.lat, obs.lon, obs.elev);

    astro_equatorial_t eq = Astronomy_Equator(body, &t, loc, EQUATOR_OF_DATE, ABERRATION);
    if (eq.status != ASTRO_SUCCESS)
        return {0, 0, TargetStatus::AstronomyError};

    astro_horizon_t hor = Astronomy_Horizon(&t, loc, eq.ra, eq.dec, kRefraction);

    return orientation_to_mount(hor.azimuth, hor.altitude, orient);
}

bool utc_valid(const UtcTime& t) {
    if (!t.year || t.month < 1 || t.month > 12 || t.hour > 23 || t.min > 59 ||
        !isfinite(t.sec) || t.sec < 0 || t.sec >= 60)
        return false;
    const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const bool leap = t.year % 4 == 0 && (t.year % 100 != 0 || t.year % 400 == 0);
    return t.day >= 1 && t.day <= days[t.month - 1] + (t.month == 2 && leap);
}
