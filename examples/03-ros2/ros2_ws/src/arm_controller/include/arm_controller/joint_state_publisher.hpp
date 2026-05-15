#pragma once
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <array>
#include <string>
#include <vector>

namespace arm_controller {

class JointStatePublisher : public rclcpp::Node {
public:
    explicit JointStatePublisher(const rclcpp::NodeOptions& options);
    void set_joint_positions(const std::array<double, 6>& positions);
    std::array<double, 6> get_joint_positions() const { return positions_; }

private:
    void timer_callback();
    rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr pub_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::array<double, 6> positions_{};
    const std::vector<std::string> joint_names_{
        "shoulder_pan","shoulder_lift","elbow","wrist_1","wrist_2","wrist_3"};
};

}  // namespace arm_controller
