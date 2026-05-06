#include "sensor_fusion/imu_simulator.hpp"
#include <cmath>

namespace sensor_fusion {

ImuSimulator::ImuSimulator(const rclcpp::NodeOptions& opts)
    : rclcpp::Node("imu_simulator", opts)
{
    declare_parameter("noise_stddev_accel", 0.01);
    declare_parameter("noise_stddev_gyro",  0.001);
    accel_noise_ = std::normal_distribution<double>(0.0, get_parameter("noise_stddev_accel").as_double());
    gyro_noise_  = std::normal_distribution<double>(0.0, get_parameter("noise_stddev_gyro").as_double());

    // BEST_EFFORT QoS — models real IMU (lossy acceptable)
    auto qos = rclcpp::QoS(rclcpp::KeepLast(10)).best_effort();
    pub_ = create_publisher<custom_interfaces::msg::ImuReading>("/imu", qos);

    timer_ = create_wall_timer(
        std::chrono::milliseconds(10),  // 100 Hz
        std::bind(&ImuSimulator::timer_callback, this));
}

void ImuSimulator::timer_callback() {
    sim_time_ += 0.01;
    auto msg = custom_interfaces::msg::ImuReading{};
    msg.header.stamp    = now();
    msg.header.frame_id = "imu_link";

    msg.angular_velocity[2] = 0.1 + gyro_noise_(rng_);
    msg.linear_acceleration[0] = 0.5 + accel_noise_(rng_);
    msg.linear_acceleration[1] = accel_noise_(rng_);
    msg.linear_acceleration[2] = 9.81 + accel_noise_(rng_);

    pub_->publish(msg);
}

}  // namespace sensor_fusion
