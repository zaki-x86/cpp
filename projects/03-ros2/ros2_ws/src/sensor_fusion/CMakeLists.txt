cmake_minimum_required(VERSION 3.8)
project(sensor_fusion)
set(CMAKE_CXX_STANDARD 17)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(custom_interfaces REQUIRED)

include_directories(include)

# State estimator pure-logic library (no ROS — unit-testable)
add_library(estimator_lib src/state_estimator_logic.cpp)
target_include_directories(estimator_lib PUBLIC include)

foreach(node imu_simulator lidar_simulator diagnostics_aggregator)
  add_executable(${node} src/${node}.cpp src/${node}_main.cpp)
  ament_target_dependencies(${node} rclcpp sensor_msgs diagnostic_msgs custom_interfaces)
endforeach()

add_executable(state_estimator src/state_estimator.cpp src/state_estimator_main.cpp)
ament_target_dependencies(state_estimator rclcpp custom_interfaces)
target_link_libraries(state_estimator estimator_lib)

install(TARGETS imu_simulator lidar_simulator state_estimator diagnostics_aggregator
  DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_state_estimator test/test_state_estimator.cpp)
  target_link_libraries(test_state_estimator estimator_lib)
endif()

ament_package()
