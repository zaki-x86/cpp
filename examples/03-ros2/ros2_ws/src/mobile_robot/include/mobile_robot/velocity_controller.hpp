#pragma once
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <custom_interfaces/srv/set_velocity.hpp>

namespace mobile_robot {

class VelocityController : public rclcpp_lifecycle::LifecycleNode {
public:
    explicit VelocityController(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;
    CallbackReturn on_configure(const rclcpp_lifecycle::State&) override;
    CallbackReturn on_activate(const rclcpp_lifecycle::State&) override;
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State&) override;
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State&) override;

private:
    void set_velocity_callback(
        const std::shared_ptr<custom_interfaces::srv::SetVelocity::Request> req,
        std::shared_ptr<custom_interfaces::srv::SetVelocity::Response> res);
    void timer_callback();

    rclcpp_lifecycle::LifecyclePublisher<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_pub_;
    rclcpp::Service<custom_interfaces::srv::SetVelocity>::SharedPtr set_vel_srv_;
    rclcpp::TimerBase::SharedPtr timer_;

    double max_linear_{1.0}, max_angular_{1.5};
    double target_linear_{0.0}, target_angular_{0.0};
};

}  // namespace mobile_robot
