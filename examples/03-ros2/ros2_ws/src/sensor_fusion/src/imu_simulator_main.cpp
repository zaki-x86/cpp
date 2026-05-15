#include "sensor_fusion/imu_simulator.hpp"
#include <rclcpp/rclcpp.hpp>
int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<sensor_fusion::ImuSimulator>());
    rclcpp::shutdown();
    return 0;
}
