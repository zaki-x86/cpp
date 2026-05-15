#include "sensor_fusion/state_estimator.hpp"
#include <rclcpp/rclcpp.hpp>
int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    // MultiThreadedExecutor to run both callback groups concurrently
    rclcpp::executors::MultiThreadedExecutor exec;
    auto node = std::make_shared<sensor_fusion::StateEstimator>();
    exec.add_node(node);
    exec.spin();
    rclcpp::shutdown();
    return 0;
}
