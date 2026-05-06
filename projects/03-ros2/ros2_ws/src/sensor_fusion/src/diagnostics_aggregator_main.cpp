#include "sensor_fusion/diagnostics_aggregator.hpp"
#include <rclcpp/rclcpp.hpp>
int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<sensor_fusion::DiagnosticsAggregator>());
    rclcpp::shutdown();
    return 0;
}
