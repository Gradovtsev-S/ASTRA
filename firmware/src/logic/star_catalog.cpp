#include "star_catalog.h"
#include <stddef.h>


// 


struct StarEntry{
    const char* name;
    double ra_h;
    double dec_deg;
};
static const StarEntry kStars[] = {
    { "Sirius",     6.7525,  -16.7161 },
    { "Arcturus",  14.2610,   19.1825 },
    { "Vega",      18.6156,   38.7837 },
    { "Capella",    5.2782,   45.9980 },
    { "Rigel",      5.2423,   -8.2016 },
    { "Betelgeuse", 5.9195,    7.4071 },
    { "Altair",    19.8464,    8.8683 },
    { "Antares",   16.4901,  -26.4320 },
    { "Deneb",     20.6905,   45.2803 },
    { "Polaris",    2.5303,   89.2641 },
};
static char lower_ascii(char c){
    return (c >= 'A' && c <= 'Z') ? (char)(c - 'A' + 'a') : c;
}
static bool equals_nocase(const char* a, const char* b){
    while (*a && *b) {
        if (lower_ascii(*a) != lower_ascii(*b)) return false;
            ++a; 
            ++b;
    }
    return *a == *b;
}
bool star_catalog_find(const char* name, double* ra_h, double* dec_deg){
    if (name == NULL || ra_h == NULL || dec_deg == NULL)
        return false;
    for (size_t i = 0; i < sizeof(kStars) / sizeof(kStars[0]); ++i){
        if (equals_nocase(name, kStars[i].name)){
            *ra_h = kStars[i].ra_h;
            *dec_deg = kStars[i].dec_deg;
            return true;
        }
    }
    return false;
}