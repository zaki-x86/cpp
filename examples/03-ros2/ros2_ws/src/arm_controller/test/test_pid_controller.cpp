#include <gtest/gtest.h>
#include "arm_controller/pid_logic.hpp"

using arm_controller::PidLoop;
using arm_controller::PidGains;

TEST(PidLoop, ProportionalOnly_StepResponse) {
    PidLoop pid({1.0, 0.0, 0.0});
    // setpoint=1, actual=0 → error=1 → output=1
    double out = pid.update(1.0, 0.0, 0.1);
    EXPECT_DOUBLE_EQ(out, 1.0);
}

TEST(PidLoop, IntegralAccumulates) {
    PidLoop pid({0.0, 1.0, 0.0});  // pure I
    // 10 steps of error=1 at dt=0.1 → integral=1.0 → output=1.0
    double out = 0.0;
    for (int i = 0; i < 10; ++i) out = pid.update(1.0, 0.0, 0.1);
    EXPECT_NEAR(out, 1.0, 1e-9);
}

TEST(PidLoop, DerivativeKickOnStep) {
    PidLoop pid({0.0, 0.0, 1.0});  // pure D
    pid.update(1.0, 0.0, 0.1);    // first step: error=1, prev=0 → deriv=10
    double out = pid.update(1.0, 0.0, 0.1);  // error still 1, prev=1 → deriv=0
    EXPECT_NEAR(out, 0.0, 1e-9);
}

TEST(PidLoop, ResetClearsIntegral) {
    PidLoop pid({0.0, 1.0, 0.0});
    for (int i = 0; i < 5; ++i) pid.update(1.0, 0.0, 0.1);
    pid.reset();
    double out = pid.update(1.0, 0.0, 0.1);  // integral cleared → output = 1*0.1 = 0.1
    EXPECT_NEAR(out, 0.1, 1e-9);
}

TEST(PidLoop, SetGainsResetsState) {
    PidLoop pid({0.0, 1.0, 0.0});
    for (int i = 0; i < 5; ++i) pid.update(1.0, 0.0, 0.1);
    pid.set_gains({1.0, 0.0, 0.0});
    EXPECT_DOUBLE_EQ(pid.gains().kp, 1.0);
    double out = pid.update(1.0, 0.0, 0.1);
    EXPECT_NEAR(out, 1.0, 1e-9);  // pure P now, integral cleared
}
