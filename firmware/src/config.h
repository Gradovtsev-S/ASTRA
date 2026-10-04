#pragma once

// ============================================================
//  ASTRA: конфигурация прошивки (ESP32)
//  Все значения ниже - заготовки. Пины -1 = не назначен.
//  Wi-Fi и токен Telegram лежат в secrets.h
// ============================================================

// ---------- Моторы (TMC2209 x2) ----------
#define MOTOR_AZ_STEP_PIN        -1
#define MOTOR_AZ_DIR_PIN         -1
#define MOTOR_AZ_EN_PIN          -1

#define MOTOR_ALT_STEP_PIN       -1
#define MOTOR_ALT_DIR_PIN        -1
#define MOTOR_ALT_EN_PIN         -1

// Направление вращения: 1 или -1 (инверсия, если ось крутится не туда)
#define MOTOR_AZ_DIR_SIGN        0
#define MOTOR_ALT_DIR_SIGN       0

// Механика: шагов на оборот мотора, микрошаг, редукция до оси
#define MOTOR_STEPS_PER_REV      0
#define MOTOR_MICROSTEPS         0
#define MOTOR_AZ_GEAR_RATIO      0
#define MOTOR_ALT_GEAR_RATIO     0

// Динамика (шагов/с и шагов/с^2)
#define MOTOR_MAX_SPEED          0
#define MOTOR_ACCEL              0

// Допуск «цель достигнута», градусы
#define MOTOR_TARGET_TOLERANCE_DEG  0

// ---------- Поиск нуля (датчики Холла A3144E) ----------
#define HALL_AZ_PIN              -1
#define HALL_ALT_PIN             -1
#define HALL_ACTIVE_LEVEL        0      // 0 или 1: уровень при срабатывании

#define HOMING_SPEED             0      // шагов/с
#define HOMING_TIMEOUT_MS        0

// Угол оси в момент срабатывания Холла (в системе монтировки, градусы)
#define HOME_AZ_OFFSET_DEG       0
#define HOME_ALT_OFFSET_DEG      0

// ---------- Лазер ----------
#define LASER_PIN                -1
#define LASER_ACTIVE_LEVEL       0      // 0 или 1
#define LASER_MAX_ON_MS          0      // автоотключение

// ---------- I2C (IMU, магнитометр, RTC) ----------
#define I2C_PORT                 0
#define I2C_SDA_PIN              -1
#define I2C_SCL_PIN              -1
#define I2C_FREQ_HZ              0

#define IMU_I2C_ADDR             0      // BMI160
#define MAG_I2C_ADDR             0      // GY-511 (магнитометр)
#define MAG_ACCEL_I2C_ADDR       0      // GY-511 (акселерометр)
#define RTC_I2C_ADDR             0      // DS3231

// ---------- GPS (UART) ----------
#define GPS_UART_NUM             0
#define GPS_TX_PIN               -1
#define GPS_RX_PIN               -1
#define GPS_BAUD                 0

// ---------- Компас ----------
// Магнитное склонение, градусы, + к востоку
#define MAG_DECLINATION_DEG      0.0f

// Угол между направлением az = 0 монтировки и осью X датчика, градусы
#define MAG_MOUNT_OFFSET_DEG     0.0f

// ---------- IMU ----------
// Поправки нуля наклона основания, градусы
#define IMU_ROLL_OFFSET_DEG      0.0f
#define IMU_PITCH_OFFSET_DEG     0.0f

// ---------- Логика наведения ----------
// Минимальная высота цели над горизонтом для включения лазера, градусы
#define MIN_ELEVATION_DEG        0.0f

// Периоды, мс
#define SENSORS_POLL_PERIOD_MS   0
#define TRACKING_PERIOD_MS       0

// ---------- Задачи FreeRTOS ----------
#define TASK_SENSORS_STACK       0
#define TASK_SENSORS_PRIO        0
#define TASK_MOTORS_STACK        0
#define TASK_MOTORS_PRIO         0
#define TASK_TRACKING_STACK      0
#define TASK_TRACKING_PRIO       0