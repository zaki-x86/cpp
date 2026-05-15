from launch import LaunchDescription
from launch_ros.actions import Node, LifecycleNode
from launch.actions import TimerAction, EmitEvent, RegisterEventHandler
from launch_ros.events.lifecycle import ChangeState
from launch_ros.event_handlers import OnStateTransition
from lifecycle_msgs.msg import Transition


def generate_launch_description():
    map_publisher = LifecycleNode(
        package='mobile_robot', executable='map_publisher',
        name='map_publisher', namespace='', output='screen')

    odometry_node = LifecycleNode(
        package='mobile_robot', executable='odometry_node',
        name='odometry_node', namespace='', output='screen')

    velocity_controller = LifecycleNode(
        package='mobile_robot', executable='velocity_controller',
        name='velocity_controller', namespace='', output='screen',
        parameters=[{'max_linear_vel': 1.0, 'max_angular_vel': 1.5}])

    path_planner = Node(
        package='mobile_robot', executable='path_planner_server',
        name='path_planner_server', output='screen')

    # Configure all lifecycle nodes after 1s, activate after 2s
    configure_map = TimerAction(period=1.0, actions=[
        EmitEvent(event=ChangeState(
            lifecycle_node_matcher=lambda node: node is map_publisher,
            transition_id=Transition.TRANSITION_CONFIGURE))])

    activate_map = TimerAction(period=2.0, actions=[
        EmitEvent(event=ChangeState(
            lifecycle_node_matcher=lambda node: node is map_publisher,
            transition_id=Transition.TRANSITION_ACTIVATE))])

    configure_odom = TimerAction(period=1.0, actions=[
        EmitEvent(event=ChangeState(
            lifecycle_node_matcher=lambda node: node is odometry_node,
            transition_id=Transition.TRANSITION_CONFIGURE))])

    activate_odom = TimerAction(period=2.0, actions=[
        EmitEvent(event=ChangeState(
            lifecycle_node_matcher=lambda node: node is odometry_node,
            transition_id=Transition.TRANSITION_ACTIVATE))])

    configure_vc = TimerAction(period=1.0, actions=[
        EmitEvent(event=ChangeState(
            lifecycle_node_matcher=lambda node: node is velocity_controller,
            transition_id=Transition.TRANSITION_CONFIGURE))])

    activate_vc = TimerAction(period=2.0, actions=[
        EmitEvent(event=ChangeState(
            lifecycle_node_matcher=lambda node: node is velocity_controller,
            transition_id=Transition.TRANSITION_ACTIVATE))])

    return LaunchDescription([
        map_publisher, odometry_node, velocity_controller, path_planner,
        configure_map, activate_map,
        configure_odom, activate_odom,
        configure_vc, activate_vc,
    ])
