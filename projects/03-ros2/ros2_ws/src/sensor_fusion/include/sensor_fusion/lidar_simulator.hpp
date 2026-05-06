#pragma once
#include <rclcpp/rclcpp.hpp>
#include <custom_interfaces/msg/point_cloud2_d.hpp>

namespace sensor_fusion {

class LidarSimulator : public rclcpp::Node {
public:
    explicit LidarSimulator(const rclcpp::NodeOptions& = rclcpp::NodeOptions());
private:
    void timer_callback();
    rclcpp::Publisher<custom_interfaces::msg::PointCloud2D>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace sensor_fusion
