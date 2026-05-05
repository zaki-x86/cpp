#include "mobile_robot/velocity_controller.hpp"
#include <algorithm>

namespace mobile_robot {

VelocityController::VelocityController(const rclcpp::NodeOptions& options)
    : rclcpp_lifecycle::LifecycleNode("velocity_controller", options) {}

VelocityController::CallbackReturn VelocityController::on_configure(const rclcpp_lifecycle::State&) {
    declare_parameter("max_linear_vel",  1.0);
    declare_parameter("max_angular_vel", 1.5);
    max_linear_  = get_parameter("max_linear_vel").as_double();
    max_angular_ = get_parameter("max_angular_vel").as_double();

    cmd_vel_pub_ = create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
    set_vel_srv_ = create_service<custom_interfaces::srv::SetVelocity>(
        "/set_velocity",
        std::bind(&VelocityController::set_velocity_callback, this,
                  std::placeholders::_1, std::placeholders::_2));
    return CallbackReturn::SUCCESS;
}

VelocityController::CallbackReturn VelocityController::on_activate(const rclcpp_lifecycle::State&) {
    cmd_vel_pub_->on_activate();
    timer_ = create_wall_timer(
        std::chrono::milliseconds(50),
        std::bind(&VelocityController::timer_callback, this));
    return CallbackReturn::SUCCESS;
}

VelocityController::CallbackReturn VelocityController::on_deactivate(const rclcpp_lifecycle::State&) {
    timer_->cancel();
    cmd_vel_pub_->on_deactivate();
    return CallbackReturn::SUCCESS;
}

VelocityController::CallbackReturn VelocityController::on_cleanup(const rclcpp_lifecycle::State&) {
    cmd_vel_pub_.reset(); set_vel_srv_.reset(); timer_.reset();
    return CallbackReturn::SUCCESS;
}

void VelocityController::set_velocity_callback(
    const std::shared_ptr<custom_interfaces::srv::SetVelocity::Request> req,
    std::shared_ptr<custom_interfaces::srv::SetVelocity::Response> res)
{
    double lin = std::clamp(req->linear,  -max_linear_,  max_linear_);
    double ang = std::clamp(req->angular, -max_angular_, max_angular_);
    if (lin != req->linear || ang != req->angular) {
        res->accepted = false;
        res->reason   = "Velocity clamped to limits";
    } else {
        res->accepted = true;
        res->reason   = "";
    }
    target_linear_  = lin;
    target_angular_ = ang;
}

void VelocityController::timer_callback() {
    geometry_msgs::msg::Twist twist{};
    twist.linear.x  = target_linear_;
    twist.angular.z = target_angular_;
    cmd_vel_pub_->publish(twist);
}

}  // namespace mobile_robot
