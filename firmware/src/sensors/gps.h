#pragma once
#include <stdint.h>
#include <stdbool.h>

struct GpsData {
    double   lat;         // широта, градусы (+ север)
    double   lon;         // долгота, градусы (+ восток)
    double   elev;        // высота над уровнем моря, м
    uint16_t year;
    uint8_t  month, day;
    uint8_t  hour, min, sec;   // UTC
    bool     pos_valid;   // координаты известны (с GPS, ранее сохранённые или ручные)
    bool     time_valid;  // время достоверно (GPS или RTC)
};

// UART для Neoway G7A, I2C для DS3231
bool gps_init(void);

// Потокобезопасно. При потере спутников время берётся из RTC,
// последняя известная позиция сохраняется (pos_valid остаётся true).
GpsData gps_get_data(void);

// Ручной ввод координат, выставляет pos_valid = true
void gps_set_manual(double lat, double lon, double elev);