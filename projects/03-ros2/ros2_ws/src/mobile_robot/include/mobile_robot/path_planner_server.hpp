#pragma once
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <custom_interfaces/action/navigate_to.hpp>
#include <custom_interfaces/srv/set_velocity.hpp>
#include <custom_interfaces/msg/odometry2_d.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include "mobile_robot/a_star.hpp"
#include <atomic>
#include <mutex>

namespace mobile_robot {

class PathPlannerServer : public rclcpp::Node {
public:
    using NavigateTo = custom_interfaces::action::NavigateTo;
    using GoalHandle = rclcpp_action::ServerGoalHandle<NavigateTo>;

    explicit PathPlannerServer(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

private:
    rclcpp_action::GoalResponse handle_goal(
        const rclcpp_action::GoalUUID& uuid,
        std::shared_ptr<const NavigateTo::Goal> goal);

    rclcpp_action::CancelResponse handle_cancel(
        const std::shared_ptr<GoalHandle> handle);

    void handle_accepted(const std::shared_ptr<GoalHandle> handle);
    void execute(const std::shared_ptr<GoalHandle> handle);

    double distance(double x1, double y1, double x2, double y2);

    rclcpp_action::Server<NavigateTo>::SharedPtr action_server_;
    rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr map_sub_;
    rclcpp::Subscription<custom_interfaces::msg::Odometry2D>::SharedPtr odom_sub_;
    rclcpp::Client<custom_interfaces::srv::SetVelocity>::SharedPtr vel_client_;

    nav_msgs::msg::OccupancyGrid::SharedPtr current_map_;
    custom_interfaces::msg::Odometry2D::SharedPtr current_odom_;
    std::atomic<bool> cancel_requested_{false};
    std::mutex map_mutex_;
    std::mutex odom_mutex_;
};

}  // namespace mobile_robot
