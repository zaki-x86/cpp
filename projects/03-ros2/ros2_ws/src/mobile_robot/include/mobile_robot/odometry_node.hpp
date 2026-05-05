#pragma once
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/msg/twist.hpp>
#include <custom_interfaces/msg/odometry2_d.hpp>
#include <memory>

namespace mobile_robot {

class OdometryNode : public rclcpp_lifecycle::LifecycleNode {
public:
    explicit OdometryNode(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;
    CallbackReturn on_configure(const rclcpp_lifecycle::State&) override;
    CallbackReturn on_activate(const rclcpp_lifecycle::State&) override;
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State&) override;
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State&) override;

private:
    void cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg);
    void timer_callback();

    rclcpp_lifecycle::LifecyclePublisher<custom_interfaces::msg::Odometry2D>::SharedPtr odom_pub_;
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_sub_;
    rclcpp::TimerBase::SharedPtr timer_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

    double x_{0.0}, y_{0.0}, theta_{0.0};
    double vx_{0.0}, vtheta_{0.0};
    rclcpp::Time last_time_;
};

}  // namespace mobile_robot
