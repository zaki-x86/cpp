#include "sensor_fusion/state_estimator.hpp"

namespace sensor_fusion {

StateEstimator::StateEstimator(const rclcpp::NodeOptions& opts)
    : rclcpp::Node("state_estimator", opts)
{
    declare_parameter("alpha", 0.98);
    filter_.set_alpha(get_parameter("alpha").as_double());

    // Two MutuallyExclusiveCallbackGroups — each subscription on its own group
    auto imu_group  = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    auto odom_group = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    rclcpp::SubscriptionOptions imu_opts, odom_opts;
    imu_opts.callback_group  = imu_group;
    odom_opts.callback_group = odom_group;

    auto imu_qos = rclcpp::QoS(10).best_effort();
    imu_sub_ = create_subscription<custom_interfaces::msg::ImuReading>(
        "/imu", imu_qos,
        std::bind(&StateEstimator::imu_callback, this, std::placeholders::_1),
        imu_opts);

    odom_sub_ = create_subscription<custom_interfaces::msg::Odometry2D>(
        "/odom", 10,
        std::bind(&StateEstimator::odom_callback, this, std::placeholders::_1),
        odom_opts);

    fused_pub_ = create_publisher<custom_interfaces::msg::Odometry2D>("/fused_odom", 10);
}

void StateEstimator::imu_callback(const custom_interfaces::msg::ImuReading::SharedPtr msg) {
    if (first_imu_) { last_imu_time_ = msg->header.stamp; first_imu_ = false; return; }

    rclcpp::Time t(msg->header.stamp);
    double dt = (t - last_imu_time_).seconds();
    last_imu_time_ = t;
    if (dt <= 0.0 || dt > 1.0) return;

    double theta_fused = filter_.update_imu(msg->angular_velocity[2], dt);

    auto out = custom_interfaces::msg::Odometry2D{};
    out.header.stamp    = msg->header.stamp;
    out.header.frame_id = "odom";
    out.x     = last_x_;
    out.y     = last_y_;
    out.theta = theta_fused;
    fused_pub_->publish(out);
}

void StateEstimator::odom_callback(const custom_interfaces::msg::Odometry2D::SharedPtr msg) {
    filter_.update_odom(msg->theta);
    last_x_ = msg->x;
    last_y_ = msg->y;
}

}  // namespace sensor_fusion
