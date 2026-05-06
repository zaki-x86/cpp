#include "arm_controller/joint_state_publisher.hpp"
#include <rclcpp_components/register_node_macro.hpp>

namespace arm_controller {

JointStatePublisher::JointStatePublisher(const rclcpp::NodeOptions& opts)
    : rclcpp::Node("joint_state_publisher", opts)
{
    pub_   = create_publisher<sensor_msgs::msg::JointState>("/joint_states", 10);
    timer_ = create_wall_timer(
        std::chrono::milliseconds(20),  // 50 Hz
        std::bind(&JointStatePublisher::timer_callback, this));
}

void JointStatePublisher::set_joint_positions(const std::array<double, 6>& p) {
    positions_ = p;
}

void JointStatePublisher::timer_callback() {
    sensor_msgs::msg::JointState msg{};
    msg.header.stamp = now();
    msg.name = joint_names_;
    msg.position.assign(positions_.begin(), positions_.end());
    pub_->publish(msg);
}

}  // namespace arm_controller

RCLCPP_COMPONENTS_REGISTER_NODE(arm_controller::JointStatePublisher)
