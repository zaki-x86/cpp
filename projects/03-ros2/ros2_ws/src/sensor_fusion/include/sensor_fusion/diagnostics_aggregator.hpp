#pragma once
#include <rclcpp/rclcpp.hpp>
#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <custom_interfaces/msg/imu_reading.hpp>
#include <custom_interfaces/msg/point_cloud2_d.hpp>
#include <chrono>

namespace sensor_fusion {

class DiagnosticsAggregator : public rclcpp::Node {
public:
    explicit DiagnosticsAggregator(const rclcpp::NodeOptions& = rclcpp::NodeOptions());
private:
    void imu_callback(const custom_interfaces::msg::ImuReading::SharedPtr);
    void lidar_callback(const custom_interfaces::msg::PointCloud2D::SharedPtr);
    void timer_callback();

    rclcpp::Subscription<custom_interfaces::msg::ImuReading>::SharedPtr   imu_sub_;
    rclcpp::Subscription<custom_interfaces::msg::PointCloud2D>::SharedPtr lidar_sub_;
    rclcpp::Publisher<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr   diag_pub_;
    rclcpp::TimerBase::SharedPtr timer_;

    int imu_count_{0}, lidar_count_{0};
    rclcpp::Time window_start_;
};

}  // namespace sensor_fusion
