#pragma once

namespace sensor_fusion {

// Complementary filter fusing gyroscope-integrated angle with odometry angle.
// theta_fused(t) = alpha * (theta_prev + gyro_z * dt) + (1 - alpha) * theta_odom
class ComplementaryFilter {
public:
    explicit ComplementaryFilter(double alpha = 0.98) : alpha_(alpha) {}

    // Call at each IMU update
    double update_imu(double gyro_z, double dt) {
        theta_gyro_ += gyro_z * dt;
        theta_fused_ = alpha_ * theta_gyro_ + (1.0 - alpha_) * theta_odom_;
        return theta_fused_;
    }

    // Call whenever odometry theta is updated
    void update_odom(double theta_odom) {
        theta_odom_  = theta_odom;
        theta_gyro_  = theta_fused_;  // re-anchor gyro integration
    }

    double theta() const { return theta_fused_; }
    void set_alpha(double a) { alpha_ = a; }

private:
    double alpha_;
    double theta_fused_{0.0};
    double theta_gyro_{0.0};
    double theta_odom_{0.0};
};

}  // namespace sensor_fusion
