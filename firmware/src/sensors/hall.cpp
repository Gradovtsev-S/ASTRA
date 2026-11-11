#include "hall.h"
#include "../config.h"
#include <Arduino.h>

namespace {
constexpr uint32_t kDebounceMs = 20;

struct Input {
    int pin;
    int raw = -1;
    bool stable = false;
    bool valid = false;
    uint32_t changedAt = 0;

    explicit Input(int gpio) : pin(gpio) {}

    void init() {
        raw = -1;
        valid = false;
        if (pin >= 0)
            pinMode(pin, INPUT);
    }

    void update(uint32_t now) {
        if (pin < 0)
            return;
        const int level = digitalRead(pin);
        if (raw != level) {
            raw = level;
            changedAt = now;
        }
        if (now - changedAt >= kDebounceMs) {
            stable = raw == HALL_ACTIVE_LEVEL;
            valid = true;
        }
    }

    HallReading reading() const { return {raw, stable, valid}; }
};

Input az(HALL_AZ_PIN);
Input alt(HALL_ALT_PIN);
}

void hall_init() {
    az.init();
    alt.init();
}

void hall_update() {
    const uint32_t now = millis();
    az.update(now);
    alt.update(now);
}

HallReading hall_az() { return az.reading(); }
HallReading hall_alt() { return alt.reading(); }
