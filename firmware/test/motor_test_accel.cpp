// Тест моторов с плавным разгоном (библиотека FastAccelStepper).
// Импульсы STEP формирует аппаратный блок ESP32, а не цикл с задержками,
// поэтому движение не блокирует программу и идёт ровно.
// Лазер не используется.
//
// Собирается окружением motor_test_accel (см. platformio.ini).

#include <Arduino.h>
#include <FastAccelStepper.h>
#include "../src/config.h"

// ---------- Параметры теста ----------
// Микрошаг драйвера: MS1-MS3 не подключены -> 1 (полный шаг),
// все три на 3.3 В -> 16 (для A4988).
static const int   MICROSTEP    = 16;
static const float SPEED_REV_S  = 1.0f;   // максимальная скорость, оборотов вала в секунду
static const float ACCEL_TIME_S = 0.5f;   // за сколько секунд разгоняемся до максимальной скорости
static const int   PAUSE_MS     = 1000;   // пауза между этапами

// ---------- Производные величины (от микрошага не зависят по смыслу) ----------
static const int32_t  STEPS_PER_REV = 200 * MICROSTEP;
static const uint32_t SPEED_HZ      = (uint32_t)(SPEED_REV_S * STEPS_PER_REV);  // шагов/с
static const uint32_t ACCEL         = (uint32_t)(SPEED_HZ / ACCEL_TIME_S);      // шагов/с^2

static FastAccelStepperEngine engine;
static FastAccelStepper* az  = nullptr;
static FastAccelStepper* alt = nullptr;

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

static FastAccelStepper* setup_motor(int step_pin, int dir_pin, int en_pin) {
    FastAccelStepper* s = engine.stepperConnectToPin(step_pin);
    if (!s) return nullptr;
    s->setDirectionPin(dir_pin);
    s->setEnablePin(en_pin);       // активный низкий уровень (A4988)
    s->setAutoEnable(true);        // драйвер включается только на время движения
    s->setSpeedInHz(SPEED_HZ);
    s->setAcceleration(ACCEL);
    s->setCurrentPosition(0);
    return s;
}

static bool any_running() {
    return (az && az->isRunning()) || (alt && alt->isRunning());
}

// Ждёт остановки моторов. Во время ожидания программа не заблокирована:
// здесь могли бы опрашиваться датчики, а пока просто печатаем позиции.
static void wait_stop(const char* label) {
    Serial.printf("%s\n", label);
    uint32_t t = millis();
    while (any_running()) {
        if (millis() - t >= 250) {
            Serial.printf("  AZ=%ld  ALT=%ld\n",
                          (long)az->getCurrentPosition(),
                          (long)alt->getCurrentPosition());
            t = millis();
        }
        delay(10);
    }
    Serial.printf("  стоп: AZ=%ld  ALT=%ld\n",
                  (long)az->getCurrentPosition(),
                  (long)alt->getCurrentPosition());
    delay(PAUSE_MS);
}

void setup() {
    Serial.begin(115200);
    delay(500);

    if (!pins_ok()) {
        Serial.println("Ошибка: не заполнены пины моторов в config.h (значение -1).");
        while (true) delay(1000);
    }

    engine.init();
    az  = setup_motor(MOTOR_AZ_STEP_PIN,  MOTOR_AZ_DIR_PIN,  MOTOR_AZ_EN_PIN);
    alt = setup_motor(MOTOR_ALT_STEP_PIN, MOTOR_ALT_DIR_PIN, MOTOR_ALT_EN_PIN);

    if (!az || !alt) {
        Serial.println("Ошибка: не удалось подключить моторы (пин не поддерживается).");
        while (true) delay(1000);
    }

    Serial.printf("Шагов на оборот: %ld, скорость: %lu шаг/с, ускорение: %lu шаг/с^2\n",
                  (long)STEPS_PER_REV, (unsigned long)SPEED_HZ, (unsigned long)ACCEL);
    Serial.println("Тест запущен");
}

void loop() {
    // 1. Азимут: оборот вперёд и назад
    az->moveTo(STEPS_PER_REV);
    wait_stop("1. AZ: оборот вперёд");
    az->moveTo(0);
    wait_stop("1. AZ: назад в ноль");

    // 2. Возвышение: то же
    alt->moveTo(STEPS_PER_REV);
    wait_stop("2. ALT: оборот вперёд");
    alt->moveTo(0);
    wait_stop("2. ALT: назад в ноль");

    // 3. Оба мотора одновременно
    az->moveTo(STEPS_PER_REV);
    alt->moveTo(STEPS_PER_REV);
    wait_stop("3. AZ и ALT вместе: вперёд");
    az->moveTo(0);
    alt->moveTo(0);
    wait_stop("3. AZ и ALT вместе: назад");

    // 4. Короткое движение (10 градусов вала): трапеция превращается в треугольник
    az->moveTo(STEPS_PER_REV / 36);
    wait_stop("4. AZ: короткий шаг 10 градусов");
    az->moveTo(0);
    wait_stop("4. AZ: назад");

    // 5. Смена цели на ходу: мотор плавно тормозит и разворачивается
    az->moveTo(2 * STEPS_PER_REV);
    delay(600);
    Serial.println("5. AZ: смена цели на ходу");
    az->moveTo(0);
    wait_stop("5. AZ: возврат");
}
