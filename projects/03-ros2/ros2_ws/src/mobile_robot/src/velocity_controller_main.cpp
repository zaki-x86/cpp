#include "mobile_robot/velocity_controller.hpp"
#include <rclcpp/rclcpp.hpp>
int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<mobile_robot::VelocityController>();
    rclcpp::spin(node->get_node_base_interface());
    rclcpp::shutdown();
}
