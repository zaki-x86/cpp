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

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_path_planner test/test_path_planner.cpp)
  target_link_libraries(test_path_planner a_star)
endif()

ament_package()
