
#pragma once

// ============================================================
// ASTRA — ESP32 firmware configuration
//
// -1 : GPIO is not assigned
//  0 : value is not configured (unless stated otherwise)
//
// Motors and laser must remain disabled during sensor tests.
// ============================================================


// ============================================================
// 1. MOTORS — TMC2209
// ============================================================

// AZ axis
#define MOTOR_AZ_STEP_PIN        -1
#define MOTOR_AZ_DIR_PIN         -1
#define MOTOR_AZ_EN_PIN          -1

// ALT axis
#define MOTOR_ALT_STEP_PIN       -1
#define MOTOR_ALT_DIR_PIN        -1
#define MOTOR_ALT_EN_PIN         -1

// Motor direction: +1 or -1
#define MOTOR_AZ_DIR_SIGN        0
#define MOTOR_ALT_DIR_SIGN       0

// Mechanical parameters
#define MOTOR_STEPS_PER_REV      0
#define MOTOR_MICROSTEPS         0

#define MOTOR_AZ_GEAR_RATIO      0
#define MOTOR_ALT_GEAR_RATIO     0

// Motion parameters
#define MOTOR_MAX_SPEED          0
#define MOTOR_ACCEL              0

// Target tolerance in degrees
#define MOTOR_TARGET_TOLERANCE_DEG 0


// ============================================================
// 2. HALL SENSORS — A3144E

// ============================================================

// GPIO pins must be confirmed on the assembled board.
#define HALL_AZ_PIN              27
#define HALL_ALT_PIN             14
#define HALL_ACTIVE_LEVEL        0

// Homing parameters (not configured yet)
#define HOMING_SPEED             0
#define HOMING_TIMEOUT_MS        0

// Mechanical angles at Hall activation
#define HOME_AZ_OFFSET_DEG       0.0f
#define HOME_ALT_OFFSET_DEG      0.0f


// ============================================================
// 3. LASER
// ============================================================

// Laser is disabled until integration testing.
#define LASER_PIN                -1
#define LASER_ACTIVE_LEVEL       0
#define LASER_MAX_ON_MS          0


// ============================================================
// 4. I2C BUS
// ============================================================

// ESP32 I2C configuration — verified on the bench.
#define I2C_PORT                 0

#define I2C_SDA_PIN              21
#define I2C_SCL_PIN              22

#define I2C_FREQ_HZ              100000


// ============================================================
// 5. I2C DEVICES
// ============================================================

// ADXL345 accelerometer
#define IMU_I2C_ADDR             0x53

// HMC5883L magnetometer
#define MAG_I2C_ADDR             0x1E

// Other optional devices — not installed/configured
#define MAG_ACCEL_I2C_ADDR       0
#define RTC_I2C_ADDR             0


// ============================================================
// 6. GPS — NEOWAY G7A
// ============================================================

// Hardware UART2 on ESP32
#define GPS_UART_NUM             2

// Receive NMEA from GPS TX pin.
#define GPS_RX_PIN               16

// GPS TX is disabled: receive-only mode.
#define GPS_TX_PIN               -1

// Verified GPS NMEA baud rate
#define GPS_BAUD                 9600


// ============================================================
// 7. MAGNETOMETER
// ============================================================

// Magnetic declination in degrees:
// positive = east, negative = west.
//
// Must be determined for the final observation location.
#define MAG_DECLINATION_DEG      0.0f

// Angular offset between the mounting reference
// and the sensor coordinate frame.
//
// To be determined after mechanical installation.
#define MAG_MOUNT_OFFSET_DEG     0.0f


// ============================================================
// 8. ACCELEROMETER — ADXL345
// ============================================================

// Tilt zero offsets in degrees.
// Calibration will be performed after installation.
#define IMU_ROLL_OFFSET_DEG      0.0f
#define IMU_PITCH_OFFSET_DEG     0.0f


// ============================================================
// 9. SENSOR AXIS MAPPING
// ============================================================

// Mount coordinate system (right-handed):
// X — forward along mounting azimuth = 0
// Y — right
// Z — downward
//
// Each value specifies a chip axis:
// +1 = +X, -1 = -X
// +2 = +Y, -2 = -Y
// +3 = +Z, -3 = -Z
//
// Example:
// {2, -1, 3}
// Mount X = chip +Y
// Mount Y = chip -X
// Mount Z = chip +Z
//
// Zeros indicate unknown mounting orientation.
// Do not replace them with assumed values.
//
#define ADXL_MOUNT_AXES {1, -2, -3}
#define HMC_MOUNT_AXES  {1, -2, -3}


// ============================================================
// 10. POINTING LOGIC
// ============================================================

// Minimum celestial target elevation in degrees
// for allowing laser activation.
#define MIN_ELEVATION_DEG        0.0f

// Coordinate convention:
// ALT = 0   : pointing downward
// ALT = 90  : pointing horizontally
// ALT = 180 : pointing at zenith


// ============================================================
// 11. UPDATE PERIODS
// ============================================================

// Sensor sampling period in milliseconds.
// Provisional software values.
#define SENSORS_POLL_PERIOD_MS   200

// Target tracking period in milliseconds.
// Provisional software value.
#define TRACKING_PERIOD_MS       1000


// ============================================================
// 12. FREERTOS TASK CONFIGURATION
// ============================================================

// Task settings are not configured yet.
// Set these when the FreeRTOS task structure is finalized.
#define TASK_SENSORS_STACK       0
#define TASK_SENSORS_PRIO        0

#define TASK_MOTORS_STACK        0
#define TASK_MOTORS_PRIO         0

#define TASK_TRACKING_STACK      0
#define TASK_TRACKING_PRIO       0
