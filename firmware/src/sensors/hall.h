#pragma once

struct HallReading {
    int raw;
    bool active;
    bool valid;
};

void hall_init();
void hall_update();
HallReading hall_az();
HallReading hall_alt();
