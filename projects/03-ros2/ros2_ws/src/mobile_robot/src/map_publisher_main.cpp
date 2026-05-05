#include "mobile_robot/map_publisher.hpp"
#include <rclcpp/rclcpp.hpp>
int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<mobile_robot::MapPublisher>();
    rclcpp::spin(node->get_node_base_interface());
    rclcpp::shutdown();
}
