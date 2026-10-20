#include <Arduino.h>
#include <Wire.h>

void setup() {
    Serial.begin(115200);
    delay(1000);
    Wire.begin(21, 22);   // SDA = GPIO21, SCL = GPIO22

    Serial.println("Сканирование I2C...");
    int found = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("Найдено устройство: 0x%02X\n", addr);
            found++;
        }
    }
    Serial.printf("Всего найдено: %d\n", found);
}

void loop() {}