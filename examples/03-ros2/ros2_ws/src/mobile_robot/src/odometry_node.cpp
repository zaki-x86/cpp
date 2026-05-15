#include "mobile_robot/odometry_node.hpp"
#include <tf2/LinearMath/Quaternion.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <cmath>

namespace mobile_robot {

OdometryNode::OdometryNode(const rclcpp::NodeOptions& options)
    : rclcpp_lifecycle::LifecycleNode("odometry_node", options) {}

OdometryNode::CallbackReturn OdometryNode::on_configure(const rclcpp_lifecycle::State&) {
    RCLCPP_INFO(get_logger(), "Configuring OdometryNode");
    odom_pub_ = create_publisher<custom_interfaces::msg::Odometry2D>("/odom", 10);
    cmd_sub_  = create_subscription<geometry_msgs::msg::Twist>(
        "/cmd_vel", 10,
        std::bind(&OdometryNode::cmd_vel_callback, this, std::placeholders::_1));
    tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);
    last_time_ = now();
    return CallbackReturn::SUCCESS;
}

OdometryNode::CallbackReturn OdometryNode::on_activate(const rclcpp_lifecycle::State&) {
    RCLCPP_INFO(get_logger(), "Activating OdometryNode");
    last_time_ = now();
    odom_pub_->on_activate();
    timer_ = create_wall_timer(
        std::chrono::milliseconds(20),
        std::bind(&OdometryNode::timer_callback, this));
    return CallbackReturn::SUCCESS;
}

OdometryNode::CallbackReturn OdometryNode::on_deactivate(const rclcpp_lifecycle::State&) {
    RCLCPP_INFO(get_logger(), "Deactivating OdometryNode");
    timer_->cancel();
    odom_pub_->on_deactivate();
    return CallbackReturn::SUCCESS;
}

OdometryNode::CallbackReturn OdometryNode::on_cleanup(const rclcpp_lifecycle::State&) {
    RCLCPP_INFO(get_logger(), "Cleaning up OdometryNode");
    odom_pub_.reset();
    cmd_sub_.reset();
    tf_broadcaster_.reset();
    timer_.reset();
    return CallbackReturn::SUCCESS;
}

void OdometryNode::cmd_vel_callback(const geometry_msgs::msg::Twist::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(vel_mutex_);
    vx_     = msg->linear.x;
    vtheta_ = msg->angular.z;
}

void OdometryNode::timer_callback() {
    auto now = this->now();
    double dt = (now - last_time_).seconds();
    last_time_ = now;

    double vx, vtheta;
    {
        std::lock_guard<std::mutex> lock(vel_mutex_);
        vx = vx_; vtheta = vtheta_;
    }

    // Integrate velocity into pose
    theta_ += vtheta * dt;
    x_ += vx * std::cos(theta_) * dt;
    y_ += vx * std::sin(theta_) * dt;

    // Publish odometry
    auto msg = custom_interfaces::msg::Odometry2D{};
    msg.header.stamp    = now;
    msg.header.frame_id = "odom";
    msg.x = x_; msg.y = y_; msg.theta = theta_;
    msg.vx = vx; msg.vtheta = vtheta;
    odom_pub_->publish(msg);

    // Broadcast TF odom -> base_link
    geometry_msgs::msg::TransformStamped tf{};
    tf.header.stamp    = now;
    tf.header.frame_id = "odom";
    tf.child_frame_id  = "base_link";
    tf.transform.translation.x = x_;
    tf.transform.translation.y = y_;
    tf2::Quaternion q;
    q.setRPY(0, 0, theta_);
    tf.transform.rotation.x = q.x();
    tf.transform.rotation.y = q.y();
    tf.transform.rotation.z = q.z();
    tf.transform.rotation.w = q.w();
    tf_broadcaster_->sendTransform(tf);
}

}  // namespace mobile_robot
