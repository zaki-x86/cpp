#include "arm_controller/gripper_service.hpp"
#include <rclcpp_components/register_node_macro.hpp>
#include <algorithm>
#include <thread>
#include <chrono>

namespace arm_controller {

GripperService::GripperService(const rclcpp::NodeOptions& opts)
    : rclcpp::Node("gripper_service", opts)
{
    srv_ = create_service<custom_interfaces::srv::GripperCommand>(
        "/gripper_command",
        std::bind(&GripperService::handle_command, this,
                  std::placeholders::_1, std::placeholders::_2));
}

void GripperService::handle_command(
    const std::shared_ptr<custom_interfaces::srv::GripperCommand::Request> req,
    std::shared_ptr<custom_interfaces::srv::GripperCommand::Response> res)
{
    double target = std::clamp(req->position, 0.0, 1.0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    current_position_ = target;
    res->success          = true;
    res->actual_position  = current_position_;
    RCLCPP_INFO(get_logger(), "Gripper moved to %.3f", current_position_);
}

}  // namespace arm_controller

RCLCPP_COMPONENTS_REGISTER_NODE(arm_controller::GripperService)
