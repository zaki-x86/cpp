#include "mobile_robot/path_planner_server.hpp"
#include <rclcpp/rclcpp.hpp>
int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<mobile_robot::PathPlannerServer>());
    rclcpp::shutdown();
    return 0;
}
