from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
import os
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    mobile_robot_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('mobile_robot'),
                         'launch', 'mobile_robot.launch.py')))

    imu_node = Node(
        package='sensor_fusion', executable='imu_simulator',
        name='imu_simulator', output='screen')

    lidar_node = Node(
        package='sensor_fusion', executable='lidar_simulator',
        name='lidar_simulator', output='screen')

    estimator_node = Node(
        package='sensor_fusion', executable='state_estimator',
        name='state_estimator', output='screen',
        parameters=[{'alpha': 0.98}])

    diagnostics_node = Node(
        package='sensor_fusion', executable='diagnostics_aggregator',
        name='diagnostics_aggregator', output='screen')

    arm_manager = Node(
        package='arm_controller', executable='arm_component_manager',
        name='arm_component_manager', output='screen')

    return LaunchDescription([
        mobile_robot_launch,
        imu_node, lidar_node, estimator_node, diagnostics_node,
        arm_manager,
    ])
