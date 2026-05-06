# 03-ros2: Comprehensive ROS2 Humble Showcase

Four-package ROS2 workspace demonstrating production-grade robotics architecture,
running in Docker Compose with RViz2 visualization via X11 forwarding.

## Quick Start

```bash
# Build Docker image
docker compose build

# Build ROS2 workspace
docker compose run --rm dev bash -c "cd /ros2_ws && colcon build --symlink-install"

# Run all tests
docker compose run --rm dev bash -c "cd /ros2_ws && colcon test && colcon test-result --verbose"

# Run full demo
docker compose run --rm dev ros2 launch mobile_robot full_demo.launch.py

# Open RViz2 (requires: xhost +local:docker on WSL2 host first)
docker compose run --rm rviz2
```

## Packages

| Package | Demonstrates |
|---------|-------------|
| `custom_interfaces` | .msg, .srv, .action definitions; rosidl_generate_interfaces |
| `mobile_robot` | Lifecycle nodes, TF2, A* action server, TRANSIENT_LOCAL QoS |
| `sensor_fusion` | BEST_EFFORT/deadline QoS, MutuallyExclusiveCallbackGroup, MultiThreadedExecutor |
| `arm_controller` | Composable components, intra-process comms, live parameter callbacks, action server |

## Architecture

```
sensor_fusion                    mobile_robot                  arm_controller
────────────────────             ─────────────────────         ─────────────────────
ImuSimulator (100Hz             OdometryNode (lifecycle)       JointStatePublisher
  BEST_EFFORT)  ──/imu──>       ├── /cmd_vel sub              PidController
                                └── /odom pub ──────>         TrajectoryServer
LidarSimulator (10Hz            MapPublisher (lifecycle)       GripperService
  RELIABLE+deadline) ──/lidar─> └── /map pub (TRANSIENT_LOCAL)
                                VelocityController (lifecycle)
StateEstimator ──/fused_odom──> PathPlannerServer (action)
DiagnosticsAgg ──/diagnostics──> ──────────────────────────────────────────> RViz2
```

## Interview Talking Points

| Topic | Location |
|-------|---------|
| Lifecycle state machine | `OdometryNode`, `VelocityController`, `MapPublisher` |
| Action server with feedback + cancellation | `PathPlannerServer`, `TrajectoryServer` |
| QoS (BEST_EFFORT, RELIABLE, TRANSIENT_LOCAL, deadline) | `ImuSimulator`, `LidarSimulator`, `MapPublisher` |
| Callback groups + MultiThreadedExecutor | `StateEstimator` |
| Composable components + intra-process zero-copy | `arm_component_manager.cpp` |
| Live parameter reconfiguration | `PidController` |
| TF2 broadcasting + static transforms | `OdometryNode`, `MapPublisher` |
| Custom .msg/.srv/.action | `custom_interfaces/` |

## Tests

| Package | Test file | Tests |
|---------|-----------|-------|
| `mobile_robot` | `test_path_planner.cpp` | 5 A* path planning cases |
| `sensor_fusion` | `test_state_estimator.cpp` | 5 complementary filter cases |
| `arm_controller` | `test_pid_controller.cpp` | 5 PID math cases |

## RViz2 Setup (WSL2)

```bash
# On WSL2 host (before starting Docker)
export DISPLAY=:0
xhost +local:docker

# Start nodes
docker compose run --rm dev ros2 launch mobile_robot full_demo.launch.py &

# Start RViz2 (separate terminal)
docker compose run --rm rviz2
```

## Docs

- `docs/docker-setup.md` — Dockerfile walkthrough, WSL2 X11 setup
- `docs/ros2-architecture.md` — Node graph, topic/service/action reference
- `docs/lifecycle-nodes.md` — Lifecycle state machine, annotated source
- `docs/custom-interfaces.md` — .msg/.srv/.action syntax, CMake integration
- `docs/executors-qos.md` — QoS matrix, callback groups, MultiThreadedExecutor
- `docs/tf2-and-navigation.md` — TF2 tree, A* walkthrough, action server pattern
- `docs/components-params.md` — Composable components, live param callbacks
