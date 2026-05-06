#pragma once
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <custom_interfaces/action/move_arm.hpp>
#include <atomic>

namespace arm_controller {

class TrajectoryServer : public rclcpp::Node {
public:
    using MoveArm    = custom_interfaces::action::MoveArm;
    using GoalHandle = rclcpp_action::ServerGoalHandle<MoveArm>;
    explicit TrajectoryServer(const rclcpp::NodeOptions& options);

private:
    rclcpp_action::GoalResponse   handle_goal(const rclcpp_action::GoalUUID&, std::shared_ptr<const MoveArm::Goal>);
    rclcpp_action::CancelResponse handle_cancel(std::shared_ptr<GoalHandle>);
    void handle_accepted(std::shared_ptr<GoalHandle>);
    void execute(std::shared_ptr<GoalHandle>);

    rclcpp_action::Server<MoveArm>::SharedPtr action_server_;
    std::atomic<bool> cancel_requested_{false};
};

}  // namespace arm_controller
