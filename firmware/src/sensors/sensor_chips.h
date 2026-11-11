#pragma once
#include "../logic/orientation.h"

// Вызываются из одного цикла опроса после инициализации Wire.
bool adxl345_init();
bool adxl345_read(SensorVector& acceleration);
bool hmc5883l_init();
bool hmc5883l_read(SensorVector& magnetic);
