#pragma once
#include <rclcpp/rclcpp.hpp>
#include <custom_interfaces/msg/imu_reading.hpp>
#include <random>

namespace sensor_fusion {

class ImuSimulator : public rclcpp::Node {
public:
    explicit ImuSimulator(const rclcpp::NodeOptions& = rclcpp::NodeOptions());
private:
    void timer_callback();
    rclcpp::Publisher<custom_interfaces::msg::ImuReading>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::mt19937 rng_{42};
    std::normal_distribution<double> accel_noise_{0.0, 0.01};
    std::normal_distribution<double> gyro_noise_{0.0, 0.001};
    double sim_time_{0.0};
};

}  // namespace sensor_fusion
