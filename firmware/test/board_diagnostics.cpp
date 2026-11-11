
#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include "../src/config.h"
#include "../src/sensors/sensor_chips.h"

namespace {

constexpr int ENABLE_AZ = 13;
constexpr int ENABLE_ALT = 23;
constexpr float RAD_TO_DEG_F = 57.29577951308232f;

struct Diagnostics {
    bool adxlReady = false;
    bool hmcReady = false;
    uint32_t lastReport = 0;
};

Diagnostics& diagnostics() {
    static Diagnostics state;
    return state;
}

// ---------------- Mathematics ----------------

float magnitude(const SensorVector& v) {
    return sqrtf(
        v.x * v.x +
        v.y * v.y +
        v.z * v.z
    );
}

float normalizeAngle(float degrees) {
    while (degrees < 0.0f)
        degrees += 360.0f;

    while (degrees >= 360.0f)
        degrees -= 360.0f;

    return degrees;
}

void printAcceleration(const SensorVector& a) {
    const float norm = magnitude(a);

    // These angles assume sensor axes as defined
    // in the ADXL345 coordinate frame.
    const float roll = atan2f(
        a.y,
        a.z
    ) * RAD_TO_DEG_F;

    const float pitch = atan2f(
        -a.x,
        sqrtf(a.y * a.y + a.z * a.z)
    ) * RAD_TO_DEG_F;

    Serial.printf(
        "ACC: x=%.3f y=%.3f z=%.3f g | norm=%.3f g\n",
        a.x, a.y, a.z, norm
    );

    Serial.printf(
        "TILT: roll=%.2f deg pitch=%.2f deg\n",
        roll, pitch
    );
}

void printMagnetic(const SensorVector& b) {
    const float norm = magnitude(b);

    // Only an uncalibrated sensor XY-plane angle.
    const float heading = normalizeAngle(
        atan2f(b.y, b.x) * RAD_TO_DEG_F
    );

    Serial.printf(
        "MAG: Bx=%.2f By=%.2f Bz=%.2f uT | norm=%.2f uT\n",
        b.x, b.y, b.z, norm
    );

    Serial.printf(
        "MAG XY ANGLE (uncalibrated): %.2f deg\n",
        heading
    );
}

} // namespace

void setup() {
    // Assumes active-LOW enable inputs.
    // Keep motor power physically disconnected.
    digitalWrite(ENABLE_AZ, HIGH);
    pinMode(ENABLE_AZ, OUTPUT);

    digitalWrite(ENABLE_ALT, HIGH);
    pinMode(ENABLE_ALT, OUTPUT);

    Serial.begin(115200);

    Serial.println();
    Serial.println("ASTRA SENSOR FUSION BENCH TEST");
    Serial.println("MOTOR POWER MUST BE OFF");

    Wire.setTimeOut(10);

    if (!Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ)) {
        Serial.println("I2C initialization failed");
        return;
    }

    Diagnostics& state = diagnostics();

    state.adxlReady = adxl345_init();
    state.hmcReady = hmc5883l_init();

    Serial.printf(
        "ADXL345: %s\n",
        state.adxlReady ? "READY" : "FAILED"
    );

    Serial.printf(
        "HMC5883L: %s\n",
        state.hmcReady ? "READY" : "FAILED"
    );

    delay(100);
}

void loop() {
    Diagnostics& state = diagnostics();

    const uint32_t now = millis();

    if (now - state.lastReport < 500) {
        delay(1);
        return;
    }

    state.lastReport = now;

    SensorVector acceleration;
    SensorVector magnetic;

    if (state.adxlReady) {
        if (adxl345_read(acceleration)) {
            printAcceleration(acceleration);
        } else {
            Serial.println("ADXL345 read failed");
        }
    }

    if (state.hmcReady) {
        if (hmc5883l_read(magnetic)) {
            printMagnetic(magnetic);
        } else {
            Serial.println("HMC5883L read failed");
        }
    }

    Serial.println("-------------------------");

    delay(1);
}
