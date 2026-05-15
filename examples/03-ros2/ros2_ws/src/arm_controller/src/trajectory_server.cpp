#include "arm_controller/trajectory_server.hpp"
#include <rclcpp_components/register_node_macro.hpp>
#include <thread>
#include <chrono>

namespace arm_controller {

TrajectoryServer::TrajectoryServer(const rclcpp::NodeOptions& opts)
    : rclcpp::Node("trajectory_server", opts)
{
    action_server_ = rclcpp_action::create_server<MoveArm>(
        this, "/move_arm",
        std::bind(&TrajectoryServer::handle_goal,     this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&TrajectoryServer::handle_cancel,   this, std::placeholders::_1),
        std::bind(&TrajectoryServer::handle_accepted, this, std::placeholders::_1));
    RCLCPP_INFO(get_logger(), "TrajectoryServer ready on /move_arm");
}

rclcpp_action::GoalResponse TrajectoryServer::handle_goal(
    const rclcpp_action::GoalUUID&, std::shared_ptr<const MoveArm::Goal> goal)
{
    if (goal->trajectory.points.empty()) return rclcpp_action::GoalResponse::REJECT;
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
}

rclcpp_action::CancelResponse TrajectoryServer::handle_cancel(std::shared_ptr<GoalHandle>) {
    cancel_requested_ = true;
    return rclcpp_action::CancelResponse::ACCEPT;
}

void TrajectoryServer::handle_accepted(std::shared_ptr<GoalHandle> handle) {
    cancel_requested_ = false;
    std::thread([this, handle](){ execute(handle); }).detach();
}

void TrajectoryServer::execute(std::shared_ptr<GoalHandle> handle) {
    const auto& traj = handle->get_goal()->trajectory;
    auto feedback = std::make_shared<MoveArm::Feedback>();
    auto result   = std::make_shared<MoveArm::Result>();

    size_t total = traj.points.size();
    for (size_t i = 0; i < total; ++i) {
        if (cancel_requested_ || handle->is_canceling()) {
            result->success = false; result->message = "Cancelled";
            handle->canceled(result); return;
        }

        feedback->progress     = static_cast<double>(i + 1) / total;
        feedback->current_joint = traj.joint_names.empty() ? "unknown" : traj.joint_names[0];
        handle->publish_feedback(feedback);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    result->success = true;
    result->message = "Trajectory complete";
    handle->succeed(result);
    RCLCPP_INFO(get_logger(), "Trajectory complete (%zu waypoints)", total);
}

}  // namespace arm_controller

RCLCPP_COMPONENTS_REGISTER_NODE(arm_controller::TrajectoryServer)
