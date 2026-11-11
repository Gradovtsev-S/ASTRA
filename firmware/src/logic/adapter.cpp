#include "adapter.h"
#include "../sensors/gps.h"
#include "../sensors/gyro_accel.h"
#include "../sensors/mag_compass.h"
#include <math.h>

bool adapter_read(HandlerInputs* out) {
    if (!out)
        return false;
    const GpsData gps = gps_get_data();
    gyro_accel_update();
    const GyroAccelData tilt = gyro_accel_get_data();
    mag_compass_update(tilt.valid ? tilt.roll : NAN, tilt.valid ? tilt.pitch : NAN);
    const MagCompassData compass = mag_compass_get_data();
    const UtcTime utc{gps.year, gps.month, gps.day, gps.hour, gps.min,
                      static_cast<double>(gps.sec)};
    if (!gps.pos_valid || !gps.time_valid || !utc_valid(utc) ||
        !tilt.valid || !compass.valid)
        return false;

    *out = {{gps.lat, gps.lon, gps.elev}, utc, {tilt.roll, tilt.pitch, compass.azimuth}};
    return true;
}
