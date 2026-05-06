#include "arm_controller/pid_controller.hpp"
#include <rclcpp_components/register_node_macro.hpp>

namespace arm_controller {

PidController::PidController(const rclcpp::NodeOptions& opts)
    : rclcpp::Node("pid_controller", opts)
{
    declare_pid_params();

    auto gains = read_pid_params();
    for (auto& pid : pids_) pid.set_gains(gains);

    // Live parameter reconfiguration — re-apply gains without restart
    param_cb_ = add_on_set_parameters_callback(
        [this](const std::vector<rclcpp::Parameter>& params) {
            rcl_interfaces::msg::SetParametersResult result;
            result.successful = true;
            for (const auto& p : params) {
                if (p.get_name() == "kp" || p.get_name() == "ki" || p.get_name() == "kd") {
                    auto gains = read_pid_params();
                    for (auto& pid : pids_) pid.set_gains(gains);
                    RCLCPP_INFO(get_logger(), "PID gains updated: kp=%.3f ki=%.3f kd=%.3f",
                        gains.kp, gains.ki, gains.kd);
                }
            }
            return result;
        });

    timer_ = create_wall_timer(
        std::chrono::milliseconds(20),
        std::bind(&PidController::timer_callback, this));
}

void PidController::declare_pid_params() {
    declare_parameter("kp", 2.0);
    declare_parameter("ki", 0.1);
    declare_parameter("kd", 0.05);
}

PidGains PidController::read_pid_params() {
    return {
        get_parameter("kp").as_double(),
        get_parameter("ki").as_double(),
        get_parameter("kd").as_double()};
}

void PidController::timer_callback() {
    (void)pids_;  // gains applied; outputs would drive hardware in a real system
}

}  // namespace arm_controller

RCLCPP_COMPONENTS_REGISTER_NODE(arm_controller::PidController)
