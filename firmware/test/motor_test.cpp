// Простой тест моторов: крутит каждую ось вперёд и назад.
// Без разгона, датчиков и логики наведения, только импульсы на STEP.
// Собирается отдельным окружением motor_test (см. platformio.ini),
// в основную прошивку не входит.

#include <Arduino.h>
#include "../src/config.h"

// Параметры теста (свои, не зависят от значений в config.h)
static const int TEST_STEPS    = 200;   // один оборот при полном шаге (MS1-3 не подключены)
static const int STEP_DELAY_US = 2000;  // ~250 шагов/с, около 1,25 об/с
static const int PAUSE_MS      = 1000;

static bool pins_ok() {
    const int pins[] = {
        MOTOR_AZ_STEP_PIN,  MOTOR_AZ_DIR_PIN,  MOTOR_AZ_EN_PIN,
        MOTOR_ALT_STEP_PIN, MOTOR_ALT_DIR_PIN, MOTOR_ALT_EN_PIN
    };
    for (int p : pins) {
        if (p < 0) return false;
    }
    return true;
}

static void move(int step_pin, int dir_pin, bool forward, int steps) {
    digitalWrite(dir_pin, forward ? HIGH : LOW);
    delayMicroseconds(5);
    for (int i = 0; i < steps; i++) {
        digitalWrite(step_pin, HIGH);
        delayMicroseconds(STEP_DELAY_US);
        digitalWrite(step_pin, LOW);
        delayMicroseconds(STEP_DELAY_US);
    }
}

static void test_axis(const char* name, int step_pin, int dir_pin) {
    Serial.printf("%s: вперёд\n", name);
    move(step_pin, dir_pin, true, TEST_STEPS);
    delay(PAUSE_MS);

    Serial.printf("%s: назад\n", name);
    move(step_pin, dir_pin, false, TEST_STEPS);
    delay(PAUSE_MS);
}

void setup() {
    Serial.begin(115200);
    delay(500);

    if (!pins_ok()) {
        Serial.println("Ошибка: не заполнены пины моторов в config.h (значение -1).");
        while (true) delay(1000);
    }

    const int out_pins[] = {
        MOTOR_AZ_STEP_PIN,  MOTOR_AZ_DIR_PIN,  MOTOR_AZ_EN_PIN,
        MOTOR_ALT_STEP_PIN, MOTOR_ALT_DIR_PIN, MOTOR_ALT_EN_PIN
    };
    for (int p : out_pins) pinMode(p, OUTPUT);

    // EN у A4988 активен низким уровнем: LOW = драйвер включён
    digitalWrite(MOTOR_AZ_EN_PIN,  LOW);
    digitalWrite(MOTOR_ALT_EN_PIN, LOW);

    Serial.println("Тест моторов запущен");
}

void loop() {
    test_axis("AZ",  MOTOR_AZ_STEP_PIN,  MOTOR_AZ_DIR_PIN);
    test_axis("ALT", MOTOR_ALT_STEP_PIN, MOTOR_ALT_DIR_PIN);
}