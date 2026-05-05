#include "mobile_robot/path_planner_server.hpp"
#include <cmath>
#include <thread>

namespace mobile_robot {

PathPlannerServer::PathPlannerServer(const rclcpp::NodeOptions& options)
    : rclcpp::Node("path_planner_server", options)
{
    action_server_ = rclcpp_action::create_server<NavigateTo>(
        this, "/navigate_to",
        std::bind(&PathPlannerServer::handle_goal,     this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&PathPlannerServer::handle_cancel,   this, std::placeholders::_1),
        std::bind(&PathPlannerServer::handle_accepted, this, std::placeholders::_1));

    map_sub_ = create_subscription<nav_msgs::msg::OccupancyGrid>(
        "/map", rclcpp::QoS(1).transient_local().reliable(),
        [this](nav_msgs::msg::OccupancyGrid::SharedPtr msg) {
            std::lock_guard<std::mutex> lock(map_mutex_);
            current_map_ = msg;
        });

    odom_sub_ = create_subscription<custom_interfaces::msg::Odometry2D>(
        "/odom", 10,
        [this](custom_interfaces::msg::Odometry2D::SharedPtr msg) {
            std::lock_guard<std::mutex> lock(odom_mutex_);
            current_odom_ = msg;
        });

    vel_client_ = create_client<custom_interfaces::srv::SetVelocity>("/set_velocity");

    RCLCPP_INFO(get_logger(), "PathPlannerServer ready");
}

rclcpp_action::GoalResponse PathPlannerServer::handle_goal(
    const rclcpp_action::GoalUUID&,
    std::shared_ptr<const NavigateTo::Goal> goal)
{
    RCLCPP_INFO(get_logger(), "Received goal: (%.2f, %.2f, %.2f)", goal->x, goal->y, goal->theta);
    std::lock_guard<std::mutex> lock(map_mutex_);
    if (!current_map_) {
        RCLCPP_WARN(get_logger(), "No map received yet — rejecting goal");
        return rclcpp_action::GoalResponse::REJECT;
    }
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse PathPlannerServer::handle_cancel(
    const std::shared_ptr<GoalHandle>)
{
    RCLCPP_INFO(get_logger(), "Cancel requested");
    cancel_requested_ = true;
    return rclcpp_action::CancelResponse::ACCEPT;
}

void PathPlannerServer::handle_accepted(const std::shared_ptr<GoalHandle> handle) {
    cancel_requested_ = false;
    std::thread([this, handle]() { execute(handle); }).detach();
}

void PathPlannerServer::execute(const std::shared_ptr<GoalHandle> handle) {
    const auto& goal = handle->get_goal();
    auto feedback = std::make_shared<NavigateTo::Feedback>();
    auto result   = std::make_shared<NavigateTo::Result>();

    // Snapshot map under lock
    nav_msgs::msg::OccupancyGrid::SharedPtr map_snapshot;
    {
        std::lock_guard<std::mutex> lock(map_mutex_);
        map_snapshot = current_map_;
    }

    if (!map_snapshot) {
        result->reached = false; result->message = "No map"; result->final_error = -1.0;
        handle->abort(result); return;
    }

    float res = map_snapshot->info.resolution;
    int rows = map_snapshot->info.height;
    int cols = map_snapshot->info.width;

    // Build Grid from OccupancyGrid
    Grid grid(rows, std::vector<int>(cols));
    for (int r = 0; r < rows; ++r)
        for (int c = 0; c < cols; ++c)
            grid[r][c] = (map_snapshot->data[r*cols + c] > 50) ? 100 : 0;

    Cell goal_cell = {
        static_cast<int>(goal->y / res),
        static_cast<int>(goal->x / res)};

    Cell start_cell = {1, 1};  // default if no odom
    {
        std::lock_guard<std::mutex> lock(odom_mutex_);
        if (current_odom_) {
            start_cell = {
                static_cast<int>(current_odom_->y / res),
                static_cast<int>(current_odom_->x / res)};
        }
    }

    Path path = a_star(grid, start_cell, goal_cell);

    if (path.empty()) {
        result->reached = false; result->message = "No path found"; result->final_error = -1.0;
        handle->abort(result); return;
    }

    // Follow the path
    rclcpp::Rate rate(10);
    for (size_t i = 1; i < path.size(); ++i) {
        if (cancel_requested_ || handle->is_canceling()) {
            result->reached = false; result->message = "Cancelled"; result->final_error = -1.0;
            handle->canceled(result); return;
        }

        double tx = path[i].second * res;
        double ty = path[i].first  * res;
        (void)tx; (void)ty;  // used for waypoint tracking; velocity is sent below

        double cx = 0.0, cy = 0.0;
        {
            std::lock_guard<std::mutex> lock(odom_mutex_);
            if (current_odom_) { cx = current_odom_->x; cy = current_odom_->y; }
        }

        feedback->distance_remaining = distance(cx, cy, goal->x, goal->y);
        feedback->heading_error = 0.0;
        handle->publish_feedback(feedback);

        if (vel_client_->service_is_ready()) {
            auto req = std::make_shared<custom_interfaces::srv::SetVelocity::Request>();
            req->linear = 0.3; req->angular = 0.0;
            vel_client_->async_send_request(req);
        }

        rate.sleep();
    }

    // Stop robot
    if (vel_client_->service_is_ready()) {
        auto req = std::make_shared<custom_interfaces::srv::SetVelocity::Request>();
        req->linear = 0.0; req->angular = 0.0;
        vel_client_->async_send_request(req);
    }

    double cx = 0.0, cy = 0.0;
    {
        std::lock_guard<std::mutex> lock(odom_mutex_);
        if (current_odom_) { cx = current_odom_->x; cy = current_odom_->y; }
    }
    result->reached = true;
    result->final_error = distance(cx, cy, goal->x, goal->y);
    result->message = "Goal reached";
    handle->succeed(result);
    RCLCPP_INFO(get_logger(), "Goal reached. Final error: %.3f", result->final_error);
}

double PathPlannerServer::distance(double x1, double y1, double x2, double y2) {
    return std::sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}

}  // namespace mobile_robot
