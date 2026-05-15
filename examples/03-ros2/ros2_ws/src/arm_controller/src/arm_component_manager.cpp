#include <rclcpp/rclcpp.hpp>
#include "arm_controller/joint_state_publisher.hpp"
#include "arm_controller/pid_controller.hpp"
#include "arm_controller/trajectory_server.hpp"
#include "arm_controller/gripper_service.hpp"

int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);

    // MultiThreadedExecutor: all components run concurrently in one process
    auto exec = std::make_shared<rclcpp::executors::MultiThreadedExecutor>();

    rclcpp::NodeOptions opts;
    opts.use_intra_process_comms(true);  // zero-copy between components

    auto js  = std::make_shared<arm_controller::JointStatePublisher>(opts);
    auto pid = std::make_shared<arm_controller::PidController>(opts);
    auto ts  = std::make_shared<arm_controller::TrajectoryServer>(opts);
    auto gs  = std::make_shared<arm_controller::GripperService>(opts);

    exec->add_node(js);
    exec->add_node(pid);
    exec->add_node(ts);
    exec->add_node(gs);

    RCLCPP_INFO(rclcpp::get_logger("arm_component_manager"),
        "All arm components loaded with intra-process comms");

    exec->spin();
    rclcpp::shutdown();
    return 0;
}
