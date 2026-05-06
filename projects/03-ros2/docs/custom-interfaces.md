# Custom Interfaces

The `custom_interfaces` package defines all application-specific message, service, and action
types used across the workspace. This document covers the syntax, CMake integration, C++
include paths, and complete interface definitions.

## Interface File Locations

```
custom_interfaces/
├── msg/
│   ├── ImuReading.msg
│   ├── JointPoint.msg
│   ├── JointTrajectoryCustom.msg
│   ├── Odometry2D.msg
│   └── PointCloud2D.msg
├── srv/
│   ├── GripperCommand.srv
│   └── SetVelocity.srv
└── action/
    ├── MoveArm.action
    └── NavigateTo.action
```

## Message (.msg) Syntax

A `.msg` file defines a flat struct of typed fields. Supported primitive types: `bool`,
`byte`, `char`, `float32`, `float64`, `int8`, `int16`, `int32`, `int64`, `uint8`–`uint64`,
`string`. Fixed-size arrays use `type[N]`; dynamic arrays use `type[]`.

Nested messages reference other packages with `package_name/MessageName` or bare `MessageName`
for messages in the same package.

### All Message Definitions

**`Odometry2D.msg`** — 2D robot pose and velocity from dead-reckoning:
```
std_msgs/Header header   # stamp + frame_id ("odom")
float64 x                # position x [m]
float64 y                # position y [m]
float64 theta            # heading [rad]
float64 vx               # forward velocity [m/s]
float64 vtheta           # angular velocity [rad/s]
```

**`ImuReading.msg`** — Simplified IMU reading (mirrors sensor_msgs/Imu but smaller):
```
std_msgs/Header header
float64[3] linear_acceleration   # [x, y, z] in m/s^2
float64[3] angular_velocity      # [x, y, z] in rad/s
float64[9] covariance            # 3x3 row-major covariance matrix
```

**`PointCloud2D.msg`** — 2D LiDAR scan as polar coordinates:
```
std_msgs/Header header
float64[] ranges     # range per ray [m]
float64[] angles     # angle per ray [rad]
uint32 num_points    # total number of rays
```

**`JointPoint.msg`** — One waypoint in a joint-space trajectory:
```
float64[] positions                        # joint positions [rad]
float64[] velocities                       # joint velocities [rad/s]
float64[] efforts                          # joint torques [Nm]
builtin_interfaces/Duration time_from_start  # time offset from trajectory start
```

**`JointTrajectoryCustom.msg`** — Full trajectory (sequence of JointPoints):
```
std_msgs/Header header
string[] joint_names                          # ordered joint name list
custom_interfaces/JointPoint[] points         # trajectory waypoints
```

## Service (.srv) Syntax

A `.srv` file has two sections separated by `---`. The top section is the **request**; the
bottom is the **response**. The server reads the request and fills in the response.

### All Service Definitions

**`SetVelocity.srv`** — Command robot velocity (called by PathPlannerServer → VelocityController):
```
float64 linear    # desired forward velocity [m/s]
float64 angular   # desired angular velocity [rad/s]
---
bool accepted     # true if within limits; false if clamped
string reason     # empty on success; "Velocity clamped to limits" otherwise
```

**`GripperCommand.srv`** — Control gripper position:
```
float64 position    # target gripper opening [0.0 = closed, 1.0 = fully open]
float64 max_force   # maximum force [N]
---
bool success              # true if gripper reached target position
float64 actual_position   # measured position after command
```

## Action (.action) Syntax

An `.action` file has three sections separated by `---`:
1. **Goal** — what the client requests (sent once)
2. **Result** — what the server returns on completion (sent once at the end)
3. **Feedback** — periodic progress updates from server to client (sent many times)

### All Action Definitions

**`NavigateTo.action`** — Navigate mobile robot to a 2D pose:
```
# Goal
float64 x          # target x position [m]
float64 y          # target y position [m]
float64 theta      # target heading [rad]
float64 tolerance  # success radius [m]
---
# Result
bool reached          # true if goal was reached within tolerance
float64 final_error   # Euclidean distance from goal at termination
string message        # "Goal reached" | "No path found" | "Cancelled" | "No map"
---
# Feedback
float64 distance_remaining   # Euclidean distance to goal [m]
float64 heading_error        # angular error to goal [rad]
```

**`MoveArm.action`** — Execute a joint-space trajectory on the arm:
```
# Goal
custom_interfaces/JointTrajectoryCustom trajectory
---
# Result
bool success
string message   # "Trajectory complete" | "Cancelled"
---
# Feedback
float64 progress      # fraction complete [0.0, 1.0]
string current_joint  # name of joint currently being moved
```

## CMake Integration

`custom_interfaces/CMakeLists.txt` key section:

```cmake
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/ImuReading.msg"
  "msg/JointPoint.msg"
  "msg/JointTrajectoryCustom.msg"
  "msg/Odometry2D.msg"
  "msg/PointCloud2D.msg"
  "srv/GripperCommand.srv"
  "srv/SetVelocity.srv"
  "action/MoveArm.action"
  "action/NavigateTo.action"
  DEPENDENCIES std_msgs builtin_interfaces
)
```

`DEPENDENCIES` lists packages whose message types appear in the interface definitions (e.g.,
`std_msgs/Header` in every message, `builtin_interfaces/Duration` in `JointPoint`).
Omitting a dependency causes a build error when the IDL compiler tries to resolve the nested type.

`package.xml` must declare:

```xml
<buildtool_depend>rosidl_default_generators</buildtool_depend>
<exec_depend>rosidl_default_runtime</exec_depend>
<member_of_group>rosidl_interface_packages</member_of_group>
```

`member_of_group rosidl_interface_packages` registers the package with the `ament_index` so
other packages can find it with `find_package(custom_interfaces REQUIRED)`.

## C++ Include Naming Convention

ROS2's IDL generator converts CamelCase message names to snake_case header file names:

| .msg file | C++ header | C++ type |
|-----------|-----------|---------|
| `Odometry2D.msg` | `custom_interfaces/msg/odometry2_d.hpp` | `custom_interfaces::msg::Odometry2D` |
| `ImuReading.msg` | `custom_interfaces/msg/imu_reading.hpp` | `custom_interfaces::msg::ImuReading` |
| `PointCloud2D.msg` | `custom_interfaces/msg/point_cloud2_d.hpp` | `custom_interfaces::msg::PointCloud2D` |
| `JointPoint.msg` | `custom_interfaces/msg/joint_point.hpp` | `custom_interfaces::msg::JointPoint` |
| `JointTrajectoryCustom.msg` | `custom_interfaces/msg/joint_trajectory_custom.hpp` | `custom_interfaces::msg::JointTrajectoryCustom` |

The conversion rule: uppercase letter → `_` + lowercase. The digit `2` is treated as a word
boundary, so `Odometry2D` → `odometry2_d` (not `odometry_2_d`).

## Action Type Helpers

For an action `NavigateTo`, the generator creates:

```cpp
// Goal, Result, Feedback structs
custom_interfaces::action::NavigateTo::Goal
custom_interfaces::action::NavigateTo::Result
custom_interfaces::action::NavigateTo::Feedback

// Wrapped types used by rclcpp_action
custom_interfaces::action::NavigateTo::GoalHandle
// (actually: rclcpp_action::ServerGoalHandle<custom_interfaces::action::NavigateTo>)

// Sending a goal
auto goal = custom_interfaces::action::NavigateTo::Goal{};
goal.x = 5.0; goal.y = 3.0; goal.theta = 0.0; goal.tolerance = 0.5;
client->async_send_goal(goal, send_goal_opts);
```

## Verification Commands

```bash
# Show all fields of a message
ros2 interface show custom_interfaces/msg/Odometry2D
# Output:
# std_msgs/Header header
#   builtin_interfaces/Time stamp
#   string frame_id
# float64 x
# float64 y
# float64 theta
# float64 vx
# float64 vtheta

# Show a service
ros2 interface show custom_interfaces/srv/SetVelocity

# Show an action
ros2 interface show custom_interfaces/action/NavigateTo

# List all custom_interfaces types
ros2 interface list | grep custom_interfaces

# Echo a live topic
ros2 topic echo /odom
```

## Using Interfaces in Downstream Packages

In a package that consumes `custom_interfaces`:

```cmake
# CMakeLists.txt
find_package(custom_interfaces REQUIRED)
ament_target_dependencies(my_node custom_interfaces rclcpp)
```

```xml
<!-- package.xml -->
<depend>custom_interfaces</depend>
```

```cpp
// C++ include
#include <custom_interfaces/msg/odometry2_d.hpp>
#include <custom_interfaces/srv/set_velocity.hpp>
#include <custom_interfaces/action/navigate_to.hpp>

// Usage
auto pub = create_publisher<custom_interfaces::msg::Odometry2D>("/odom", 10);
auto msg = custom_interfaces::msg::Odometry2D{};
msg.header.stamp = now();
msg.x = 1.5; msg.y = 0.0; msg.theta = 0.3;
pub->publish(msg);
```
