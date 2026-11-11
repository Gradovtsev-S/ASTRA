#include "orientation.h"
#include <assert.h>
#include <math.h>

static void near(double actual, double expected) {
    assert(fabs(actual - expected) < 1e-4);
}

int main() {
    const Orientation level{0, 0, 0};
    near(orientation_to_mount(30, 0, level).alt, 90);
    near(orientation_to_mount(30, 90, level).alt, 180);
    near(orientation_to_mount(30, -90, level).alt, 0);
    near(orientation_to_mount(15, 0, {0, 0, 30}).az, 345);
    near(orientation_to_mount(0, 30, {0, 30, 0}).alt, 90);
    near(orientation_to_mount(90, -30, {30, 0, 0}).alt, 90);

    // Независимый вектор с одновременными yaw=90°, pitch=30°, roll=30°.
    // Смотрим вдоль +Y монтировки, после активного вращения это (-sqrt(3)/2, 1/4, sqrt(3)/4).
    const auto mixed = orientation_to_mount(atan2(0.25, -sqrt(3.0)/2) * 180 / M_PI,
                                            -asin(sqrt(3.0)/4) * 180 / M_PI,
                                            {30, 30, 90});
    near(mixed.az, 90);
    near(mixed.alt, 90);

    SensorVector mounted{};
    assert(orientation_map({1, 2, 3}, {2, -1, 3}, mounted));
    near(mounted.x, 2);
    near(mounted.y, -1);
    assert(!orientation_map({1, 2, 3}, {1, 2, -3}, mounted));
    assert(!orientation_map({1, 2, 3}, {1, 1, 3}, mounted));
    float roll, pitch, magnetic, heading;
    assert(orientation_tilt({0.5f, -0.4330127f, -0.75f}, roll, pitch));
    near(roll, 30);
    near(pitch, 30);
    assert(!orientation_tilt({0, 0, 0.2f}, roll, pitch));
    // Горизонтальный север в монтировке с теми же тремя поворотами.
    assert(orientation_heading({0, -43.30127f, 25}, roll, pitch, 12, magnetic, heading));
    near(magnetic, 90);
    near(heading, 102);
    assert(!orientation_heading({0, 0, 50}, 0, 0, 0, magnetic, heading));

    const Observer obs{55.94, 37.52, 180};
    const UtcTime utc{2026, 11, 11, 12, 0, 0};
    const auto moon = handler_calculate("Moon", obs, utc, level);
    assert(moon.status == TargetStatus::Ok);
    assert(moon.az >= 0 && moon.az < 360 && moon.alt >= 0 && moon.alt <= 180);
    assert(handler_calculate("x", obs, utc, level).status == TargetStatus::UnknownObject);
    assert(handler_calculate("sun", obs, {2026, 2, 29, 0, 0, 0}, level).status ==
           TargetStatus::InvalidInput);
    assert(utc_valid({2028, 2, 29, 0, 0, 0}));
}
