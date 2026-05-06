#include "sensor_fusion/diagnostics_aggregator.hpp"
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

namespace sensor_fusion {

DiagnosticsAggregator::DiagnosticsAggregator(const rclcpp::NodeOptions& opts)
    : rclcpp::Node("diagnostics_aggregator", opts), window_start_(now())
{
    auto imu_qos = rclcpp::QoS(10).best_effort();
    imu_sub_   = create_subscription<custom_interfaces::msg::ImuReading>(
        "/imu", imu_qos,
        [this](const custom_interfaces::msg::ImuReading::SharedPtr msg){ imu_callback(msg); });
    lidar_sub_ = create_subscription<custom_interfaces::msg::PointCloud2D>(
        "/lidar", 10,
        [this](const custom_interfaces::msg::PointCloud2D::SharedPtr msg){ lidar_callback(msg); });
    diag_pub_  = create_publisher<diagnostic_msgs::msg::DiagnosticArray>("/diagnostics", 10);
    timer_     = create_wall_timer(std::chrono::seconds(1),
        std::bind(&DiagnosticsAggregator::timer_callback, this));
}

void DiagnosticsAggregator::imu_callback(const custom_interfaces::msg::ImuReading::SharedPtr)
    { ++imu_count_; }

void DiagnosticsAggregator::lidar_callback(const custom_interfaces::msg::PointCloud2D::SharedPtr)
    { ++lidar_count_; }

void DiagnosticsAggregator::timer_callback() {
    auto make_status = [](const std::string& name, int actual, int expected_hz) {
        diagnostic_msgs::msg::DiagnosticStatus s;
        s.name    = name;
        s.hardware_id = "simulated";
        double ratio = static_cast<double>(actual) / expected_hz;
        if      (ratio >= 0.9) { s.level = 0; s.message = "OK"; }
        else if (ratio >= 0.5) { s.level = 1; s.message = "Rate degraded"; }
        else                   { s.level = 2; s.message = "Rate too low"; }
        diagnostic_msgs::msg::KeyValue kv_actual, kv_expected;
        kv_actual.key   = "actual_hz";
        kv_actual.value = std::to_string(actual);
        kv_expected.key   = "expected_hz";
        kv_expected.value = std::to_string(expected_hz);
        s.values.push_back(kv_actual);
        s.values.push_back(kv_expected);
        return s;
    };

    diagnostic_msgs::msg::DiagnosticArray arr;
    arr.header.stamp = now();
    arr.status.push_back(make_status("IMU",   imu_count_,   100));
    arr.status.push_back(make_status("LiDAR", lidar_count_, 10));
    diag_pub_->publish(arr);

    imu_count_ = 0; lidar_count_ = 0;
    window_start_ = now();
}

}  // namespace sensor_fusion
