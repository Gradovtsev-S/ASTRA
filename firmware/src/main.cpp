#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "logic/adapter.h"
#include "sensors/gps.h"
#include "sensors/gyro_accel.h"
#include "sensors/hall.h"
#include "sensors/mag_compass.h"

void setup() {
    Serial.begin(115200);
    hall_init();
    const bool gpsReady = gps_init();
    Wire.setTimeOut(10);
    const bool busReady = Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN, I2C_FREQ_HZ);
    const bool accelReady = busReady && gyro_accel_init();
    const bool compassReady = busReady && mag_compass_init();
    Serial.printf("ASTRA GPS UART=%d I2C=%d ADXL345=%d HMC5883L=%d\n",
                  gpsReady, busReady, accelReady, compassReady);
}

void loop() {
    static uint32_t lastReport = 0;
    hall_update();
    HandlerInputs inputs{};
    const bool valid = adapter_read(&inputs);
    const uint32_t now = millis();
    if (now - lastReport >= 1000) {
        lastReport = now;
        const auto az = hall_az();
        const auto alt = hall_alt();
        const auto tilt = gyro_accel_get_data();
        const auto compass = mag_compass_get_data();
        const auto gps = gps_get_data();
        Serial.printf("GPS pos=%d utc=%d | tilt=%d %.2f %.2f | compass=%d %.2f cal=%d"
                      " | Hall AZ=%d/%d/%d ALT=%d/%d/%d\n",
                      gps.pos_valid, gps.time_valid, tilt.valid, tilt.roll, tilt.pitch,
                      compass.valid, compass.azimuth, compass.calibrated,
                      az.raw, az.active, az.valid, alt.raw, alt.active, alt.valid);
    }
    static uint32_t lastTarget = 0;
    if (valid && now - lastTarget >= 1000) {
        lastTarget = now;
        const auto target = handler_calculate("moon", inputs.obs, inputs.time, inputs.orient);
        Serial.printf("Moon status=%d az=%.2f alt=%.2f\n",
                      static_cast<int>(target.status), target.az, target.alt);
    }
    delay(5);
}
