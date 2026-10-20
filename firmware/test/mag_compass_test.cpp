#include <Arduino.h>
#include <Wire.h>

static const uint8_t MAG_ADDR = 0x1E;
static const uint8_t ACC_ADDR = 0x19;

// Читает один регистр. Возвращает -1, если датчик не ответил.
static int readReg(uint8_t addr, uint8_t reg) {
    Wire.beginTransmission(addr);
    Wire.write(reg);                              // говорим, какой регистр читать
    if (Wire.endTransmission(false) != 0) return -1;
    if (Wire.requestFrom(addr, (uint8_t)1) != 1) return -1;
    return Wire.read();                           // забираем один байт
}

static void printReg(uint8_t addr, uint8_t reg) {
    int v = readReg(addr, reg);
    if (v < 0) Serial.printf("  регистр 0x%02X: нет ответа\n", reg);
    else       Serial.printf("  регистр 0x%02X = 0x%02X\n", reg, v);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Wire.begin(21, 22);

    Serial.println("Магнитометр (0x1E), ожидаем 0x48, 0x34, 0x33:");
    for (uint8_t reg = 0x0A; reg <= 0x0C; reg++) printReg(MAG_ADDR, reg);

    Serial.println("Акселерометр (0x19), CTRL_REG1_A, ожидаем 0x07:");
    printReg(ACC_ADDR, 0x20);
}

void loop() {}