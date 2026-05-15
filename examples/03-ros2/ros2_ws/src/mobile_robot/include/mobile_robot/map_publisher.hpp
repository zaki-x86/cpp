#pragma once
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <tf2_ros/static_transform_broadcaster.h>

namespace mobile_robot {

class MapPublisher : public rclcpp_lifecycle::LifecycleNode {
public:
    explicit MapPublisher(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    using CallbackReturn = rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;
    CallbackReturn on_configure(const rclcpp_lifecycle::State&) override;
    CallbackReturn on_activate(const rclcpp_lifecycle::State&) override;
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State&) override;
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State&) override;

private:
    nav_msgs::msg::OccupancyGrid build_map();
    void publish_static_tf();

    rclcpp_lifecycle::LifecyclePublisher<nav_msgs::msg::OccupancyGrid>::SharedPtr map_pub_;
    std::shared_ptr<tf2_ros::StaticTransformBroadcaster> static_tf_;
};

}  // namespace mobile_robot
