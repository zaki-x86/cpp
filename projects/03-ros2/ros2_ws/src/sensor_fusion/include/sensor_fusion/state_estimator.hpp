#pragma once
#include <rclcpp/rclcpp.hpp>
#include <custom_interfaces/msg/imu_reading.hpp>
#include <custom_interfaces/msg/odometry2_d.hpp>
#include "sensor_fusion/state_estimator_logic.hpp"

namespace sensor_fusion {

class StateEstimator : public rclcpp::Node {
public:
    explicit StateEstimator(const rclcpp::NodeOptions& = rclcpp::NodeOptions());
private:
    void imu_callback(const custom_interfaces::msg::ImuReading::SharedPtr msg);
    void odom_callback(const custom_interfaces::msg::Odometry2D::SharedPtr msg);

    rclcpp::Subscription<custom_interfaces::msg::ImuReading>::SharedPtr  imu_sub_;
    rclcpp::Subscription<custom_interfaces::msg::Odometry2D>::SharedPtr  odom_sub_;
    rclcpp::Publisher<custom_interfaces::msg::Odometry2D>::SharedPtr     fused_pub_;

    ComplementaryFilter filter_;
    rclcpp::Time last_imu_time_;
    bool first_imu_{true};
    double last_x_{0.0}, last_y_{0.0};
};

}  // namespace sensor_fusion
