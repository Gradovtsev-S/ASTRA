#include "orientation.h"
#include <math.h>
#include <stdlib.h>

namespace {
constexpr double kRad = 0.017453292519943295;

double wrap(double angle) {
    const double wrapped = fmod(angle, 360.0);
    return wrapped < 0 ? wrapped + 360.0 : wrapped;
}
}

bool orientation_map(SensorVector raw, SensorAxes axes, SensorVector& mounted) {
    const int indices[] = {axes.x, axes.y, axes.z};
    const float values[] = {raw.x, raw.y, raw.z};
    float result[3];
    unsigned used = 0;
    int sign = 1;
    for (int i = 0; i < 3; ++i) {
        if (indices[i] < -3 || indices[i] > 3 || indices[i] == 0)
            return false;
        const int axis = abs(indices[i]) - 1;
        if (used & (1u << axis))
            return false;
        used |= 1u << axis;
        if (indices[i] < 0)
            sign = -sign;
        for (int j = 0; j < i; ++j)
            if (abs(indices[j]) > abs(indices[i]))
                sign = -sign;
        result[i] = indices[i] < 0 ? -values[axis] : values[axis];
    }
    if (sign < 0)
        return false;
    mounted = {result[0], result[1], result[2]};
    return true;
}

bool orientation_tilt(SensorVector up, float& roll, float& pitch) {
    const double yz = hypot(up.y, up.z);
    const double norm = hypot(up.x, yz);
    if (!isfinite(norm) || norm < 0.8 || norm > 1.2 || yz < 0.01)
        return false;
    roll = atan2(-up.y, -up.z) / kRad;
    pitch = atan2(up.x, yz) / kRad;
    return true;
}

bool orientation_heading(SensorVector b, float roll, float pitch,
                         float declination, float& magnetic, float& heading) {
    const double r = roll * kRad, p = pitch * kRad;
    const double y = cos(r) * b.y - sin(r) * b.z;
    const double z = sin(r) * b.y + cos(r) * b.z;
    const double x = cos(p) * b.x + sin(p) * z;
    const double horizontal = hypot(x, y);
    if (!isfinite(horizontal) || horizontal < 1.0 || !isfinite(declination))
        return false;
    magnetic = wrap(atan2(-y, x) / kRad);
    heading = wrap(magnetic + declination);
    if (magnetic >= 360.0f) magnetic = 0;
    if (heading >= 360.0f) heading = 0;
    return true;
}

TargetAngles orientation_to_mount(double az, double elevation, const Orientation& o) {
    if (!isfinite(az) || !isfinite(elevation) || elevation < -90 || elevation > 90 ||
        !isfinite(o.heading) || !isfinite(o.roll) || !isfinite(o.pitch))
        return {0, 0, TargetStatus::InvalidInput};

    // Обратное вращение для соглашения Rz(heading) Ry(pitch) Rx(roll).
    const double a = (az - o.heading) * kRad, e = elevation * kRad;
    const double p = o.pitch * kRad, r = o.roll * kRad;
    const double x = cos(e) * cos(a), y = cos(e) * sin(a), z = -sin(e);
    const double localX = cos(p) * x - sin(p) * z;
    const double tiltedZ = sin(p) * x + cos(p) * z;
    const double localY = cos(r) * y + sin(r) * tiltedZ;
    const double localZ = -sin(r) * y + cos(r) * tiltedZ;
    const double horizontal = hypot(localX, localY);
    const double localAz = horizontal < 1e-12 ? 0 : wrap(atan2(localY, localX) / kRad);
    return {localAz, atan2(horizontal, localZ) / kRad, TargetStatus::Ok};
}
