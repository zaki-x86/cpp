cmake_minimum_required(VERSION 3.8)
project(mobile_robot)

set(CMAKE_CXX_STANDARD 17)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(custom_interfaces REQUIRED)

include_directories(include)

# A* library (pure C++, no ROS deps — easy to unit test)
add_library(a_star src/a_star.cpp)
target_include_directories(a_star PUBLIC include)

# OdometryNode
add_executable(odometry_node src/odometry_node.cpp src/odometry_node_main.cpp)
ament_target_dependencies(odometry_node rclcpp rclcpp_lifecycle tf2_ros tf2_geometry_msgs geometry_msgs custom_interfaces)

# VelocityController
add_executable(velocity_controller src/velocity_controller.cpp src/velocity_controller_main.cpp)
ament_target_dependencies(velocity_controller rclcpp rclcpp_lifecycle geometry_msgs custom_interfaces)

# MapPublisher
add_executable(map_publisher src/map_publisher.cpp src/map_publisher_main.cpp)
ament_target_dependencies(map_publisher rclcpp rclcpp_lifecycle tf2_ros nav_msgs geometry_msgs)

install(TARGETS odometry_node velocity_controller map_publisher
  DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY launch rviz DESTINATION share/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_path_planner test/test_path_planner.cpp)
  target_link_libraries(test_path_planner a_star)
endif()

ament_package()
