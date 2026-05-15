cmake_minimum_required(VERSION 3.8)
project(arm_controller)
set(CMAKE_CXX_STANDARD 17)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(custom_interfaces REQUIRED)

include_directories(include)

# PID pure logic library — unit-testable without ROS
add_library(pid_lib src/pid_logic.cpp)
target_include_directories(pid_lib PUBLIC include)

# Composable components shared library
add_library(arm_components SHARED
  src/joint_state_publisher.cpp
  src/pid_controller.cpp
  src/trajectory_server.cpp
  src/gripper_service.cpp)
ament_target_dependencies(arm_components rclcpp rclcpp_components rclcpp_action sensor_msgs custom_interfaces)
target_link_libraries(arm_components pid_lib)
rclcpp_components_register_nodes(arm_components
  "arm_controller::JointStatePublisher"
  "arm_controller::PidController"
  "arm_controller::TrajectoryServer"
  "arm_controller::GripperService")

# Component manager executable
add_executable(arm_component_manager src/arm_component_manager.cpp)
ament_target_dependencies(arm_component_manager rclcpp rclcpp_components)
target_link_libraries(arm_component_manager arm_components)

install(TARGETS arm_components arm_component_manager
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)
  ament_add_gtest(test_pid_controller test/test_pid_controller.cpp)
  target_link_libraries(test_pid_controller pid_lib)
endif()

ament_package()
