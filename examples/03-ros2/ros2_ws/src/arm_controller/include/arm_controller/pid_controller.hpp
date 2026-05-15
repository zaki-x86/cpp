#pragma once
#include <rclcpp/rclcpp.hpp>
#include "arm_controller/pid_logic.hpp"
#include <array>

namespace arm_controller {

class PidController : public rclcpp::Node {
public:
    explicit PidController(const rclcpp::NodeOptions& options);

private:
    void timer_callback();
    void declare_pid_params();
    PidGains read_pid_params();

    rclcpp::TimerBase::SharedPtr timer_;
    std::array<PidLoop, 6>       pids_;
    std::array<double, 6>        setpoints_{};
    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_cb_;
};

}  // namespace arm_controller
