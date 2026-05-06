#pragma once
#include <rclcpp/rclcpp.hpp>
#include <custom_interfaces/srv/gripper_command.hpp>

namespace arm_controller {
class GripperService : public rclcpp::Node {
public:
    explicit GripperService(const rclcpp::NodeOptions& options);
private:
    void handle_command(
        const std::shared_ptr<custom_interfaces::srv::GripperCommand::Request> req,
        std::shared_ptr<custom_interfaces::srv::GripperCommand::Response> res);
    rclcpp::Service<custom_interfaces::srv::GripperCommand>::SharedPtr srv_;
    double current_position_{0.0};
};
}
