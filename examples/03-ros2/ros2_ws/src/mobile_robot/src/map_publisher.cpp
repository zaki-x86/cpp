#include "mobile_robot/map_publisher.hpp"
#include <geometry_msgs/msg/transform_stamped.hpp>

namespace mobile_robot {

MapPublisher::MapPublisher(const rclcpp::NodeOptions& options)
    : rclcpp_lifecycle::LifecycleNode("map_publisher", options) {}

MapPublisher::CallbackReturn MapPublisher::on_configure(const rclcpp_lifecycle::State&) {
    // TRANSIENT_LOCAL QoS = latched — late subscribers get the last message
    auto qos = rclcpp::QoS(rclcpp::KeepLast(1)).transient_local().reliable();
    map_pub_    = create_publisher<nav_msgs::msg::OccupancyGrid>("/map", qos);
    static_tf_  = std::make_shared<tf2_ros::StaticTransformBroadcaster>(*this);
    return CallbackReturn::SUCCESS;
}

MapPublisher::CallbackReturn MapPublisher::on_activate(const rclcpp_lifecycle::State&) {
    map_pub_->on_activate();
    map_pub_->publish(build_map());
    publish_static_tf();
    return CallbackReturn::SUCCESS;
}

MapPublisher::CallbackReturn MapPublisher::on_deactivate(const rclcpp_lifecycle::State&) {
    map_pub_->on_deactivate();
    return CallbackReturn::SUCCESS;
}

MapPublisher::CallbackReturn MapPublisher::on_cleanup(const rclcpp_lifecycle::State&) {
    map_pub_.reset(); static_tf_.reset();
    return CallbackReturn::SUCCESS;
}

nav_msgs::msg::OccupancyGrid MapPublisher::build_map() {
    nav_msgs::msg::OccupancyGrid grid{};
    grid.header.stamp    = now();
    grid.header.frame_id = "map";
    grid.info.resolution = 0.5f;   // 0.5 m per cell
    grid.info.width  = 20;
    grid.info.height = 20;
    grid.info.origin.position.x = 0.0;
    grid.info.origin.position.y = 0.0;
    grid.info.origin.orientation.w = 1.0;

    grid.data.resize(20 * 20, 0);

    // Perimeter walls
    for (int i = 0; i < 20; ++i) {
        grid.data[i]          = 100;  // bottom row
        grid.data[19*20 + i]  = 100;  // top row
        grid.data[i*20]       = 100;  // left col
        grid.data[i*20 + 19]  = 100;  // right col
    }
    // Central obstacle
    for (int r = 8; r <= 11; ++r)
        for (int c = 8; c <= 11; ++c)
            grid.data[r*20 + c] = 100;

    return grid;
}

void MapPublisher::publish_static_tf() {
    geometry_msgs::msg::TransformStamped tf{};
    tf.header.stamp    = now();
    tf.header.frame_id = "map";
    tf.child_frame_id  = "odom";
    tf.transform.rotation.w = 1.0;  // identity
    static_tf_->sendTransform(tf);
}

}  // namespace mobile_robot
