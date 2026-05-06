#include "sensor_fusion/lidar_simulator.hpp"
#include <cmath>

namespace sensor_fusion {

LidarSimulator::LidarSimulator(const rclcpp::NodeOptions& opts)
    : rclcpp::Node("lidar_simulator", opts)
{
    // RELIABLE + deadline(150ms) QoS
    auto qos = rclcpp::QoS(rclcpp::KeepLast(5))
                   .reliable()
                   .deadline(rclcpp::Duration(0, 150'000'000));
    pub_ = create_publisher<custom_interfaces::msg::PointCloud2D>("/lidar", qos);

    timer_ = create_wall_timer(
        std::chrono::milliseconds(100),  // 10 Hz
        std::bind(&LidarSimulator::timer_callback, this));
}

void LidarSimulator::timer_callback() {
    constexpr int NUM_RAYS = 360;
    auto msg = custom_interfaces::msg::PointCloud2D{};
    msg.header.stamp    = now();
    msg.header.frame_id = "laser";
    msg.num_points      = NUM_RAYS;
    msg.ranges.resize(NUM_RAYS);
    msg.angles.resize(NUM_RAYS);

    for (int i = 0; i < NUM_RAYS; ++i) {
        double angle = i * (2.0 * M_PI / NUM_RAYS);
        double range = 5.0;
        if (std::abs(angle) < 0.3 || std::abs(angle - 2*M_PI) < 0.3)
            range = 2.0;
        msg.ranges[i] = range;
        msg.angles[i] = angle;
    }
    pub_->publish(msg);
}

}  // namespace sensor_fusion
