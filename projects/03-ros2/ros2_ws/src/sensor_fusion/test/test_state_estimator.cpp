#include <gtest/gtest.h>
#include "sensor_fusion/state_estimator_logic.hpp"
#include <cmath>

using sensor_fusion::ComplementaryFilter;

TEST(ComplementaryFilter, PureGyro_AlphaOne) {
    ComplementaryFilter f(1.0);  // trust gyro completely
    f.update_imu(1.0, 0.1);     // 1 rad/s for 0.1s = 0.1 rad
    EXPECT_NEAR(f.theta(), 0.1, 1e-9);
}

TEST(ComplementaryFilter, PureOdom_AlphaZero) {
    ComplementaryFilter f(0.0);  // trust odom completely
    f.update_odom(0.5);
    f.update_imu(100.0, 0.1);   // large gyro ignored
    EXPECT_NEAR(f.theta(), 0.5, 1e-9);
}

TEST(ComplementaryFilter, BlendedAlpha) {
    ComplementaryFilter f(0.98);
    f.update_odom(0.0);
    // After 10 steps at 1 rad/s with dt=0.01: gyro_integrated = 0.1 rad
    for (int i = 0; i < 10; ++i) f.update_imu(1.0, 0.01);
    // theta ≈ 0.98 * 0.1 + 0.02 * 0.0 = 0.098
    EXPECT_NEAR(f.theta(), 0.098, 1e-6);
}

TEST(ComplementaryFilter, OdomUpdateReanchorsGyro) {
    ComplementaryFilter f(0.98);
    f.update_odom(1.0);
    f.update_imu(0.0, 0.1);
    // With no gyro rotation, theta should be close to odom
    EXPECT_NEAR(f.theta(), 0.02, 1e-6);  // 0.98*0 + 0.02*1.0 = 0.02
}

TEST(ComplementaryFilter, SetAlpha) {
    ComplementaryFilter f(0.5);
    f.set_alpha(1.0);
    f.update_imu(1.0, 1.0);
    EXPECT_NEAR(f.theta(), 1.0, 1e-9);
}
