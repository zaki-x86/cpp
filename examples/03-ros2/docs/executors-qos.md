# Executors, Callback Groups, and QoS

This document covers ROS2 Quality of Service (QoS) policies, callback group types,
the MultiThreadedExecutor pattern, and how the showcase uses them to demonstrate
production-grade concurrency and reliability.

## QoS Policy Matrix

ROS2 QoS builds on DDS QoS policies. The most important axes for pub/sub matching are
**Reliability** and **Durability**.

| Reliability | Durability | Description | Use When |
|-------------|-----------|-------------|----------|
| `RELIABLE` | `VOLATILE` | Retransmit lost messages; history not stored | Default for most topics |
| `BEST_EFFORT` | `VOLATILE` | No retransmit; fast as UDP | High-rate sensor data (IMU, cameras) |
| `RELIABLE` | `TRANSIENT_LOCAL` | Retransmit; store history for late joiners | Maps, configuration data |
| `BEST_EFFORT` | `TRANSIENT_LOCAL` | Rarely used; store history but don't retransmit | N/A |

### Publisher/Subscriber Compatibility Rules

For a connection to form, the subscriber's QoS must be **less demanding or equal** to the publisher's:

| Publisher | Subscriber | Connection Forms? |
|-----------|-----------|------------------|
| RELIABLE | RELIABLE | Yes |
| RELIABLE | BEST_EFFORT | Yes (sub gets all messages) |
| BEST_EFFORT | RELIABLE | **No** — subscriber demands guarantees pub cannot provide |
| BEST_EFFORT | BEST_EFFORT | Yes |
| TRANSIENT_LOCAL | TRANSIENT_LOCAL | Yes (late subscriber gets cached history) |
| TRANSIENT_LOCAL | VOLATILE | Yes (sub receives future messages only) |
| VOLATILE | TRANSIENT_LOCAL | **No** — sub expects history pub cannot provide |

When a connection fails to form due to QoS incompatibility, `ros2 topic info /topic --verbose`
shows `"0 matched"` on one or both sides with no error message — this is a common source of
hard-to-debug "why isn't my subscriber receiving anything?" problems.

## QoS in This Showcase

### ImuSimulator — BEST_EFFORT at 100 Hz

```cpp
// From imu_simulator.cpp
auto qos = rclcpp::QoS(rclcpp::KeepLast(10)).best_effort();
pub_ = create_publisher<custom_interfaces::msg::ImuReading>("/imu", qos);

// Timer fires every 10 ms
timer_ = create_wall_timer(
    std::chrono::milliseconds(10),   // 100 Hz
    std::bind(&ImuSimulator::timer_callback, this));
```

**Why BEST_EFFORT?** An IMU publishes 100 messages per second. Retransmitting a single dropped
packet adds latency and may cause the retransmit queue to grow unbounded. For state estimation,
receiving 99% of IMU packets at correct timing is better than receiving 100% with jitter.
The complementary filter in `StateEstimator` is designed to tolerate missing samples — it
checks `if (dt <= 0.0 || dt > 1.0) return;` to discard stale data.

### LidarSimulator — RELIABLE + Deadline

```cpp
// From lidar_simulator.cpp
auto qos = rclcpp::QoS(rclcpp::KeepLast(5))
               .reliable()
               .deadline(rclcpp::Duration(0, 150'000'000));  // 150 ms
pub_ = create_publisher<custom_interfaces::msg::PointCloud2D>("/lidar", qos);
```

**Why RELIABLE?** LiDAR scans are used for obstacle detection. Missing a scan for 150 ms
could mean the robot drives into an obstacle, so every scan must be delivered. The `deadline`
policy triggers a `on_offered_deadline_missed()` callback on the publisher and
`on_requested_deadline_missed()` on the subscriber if 150 ms pass without a new message.
This provides a watchdog mechanism: the DiagnosticsAggregator can report a LiDAR fault.

**Why 150 ms deadline for a 10 Hz sensor?** The LiDAR fires at 100 ms intervals. A 150 ms
deadline means one missed scan triggers the deadline event. A 200 ms deadline would require
two consecutive misses. Adjust the threshold based on safety requirements.

### MapPublisher — TRANSIENT_LOCAL ("latched")

```cpp
// From map_publisher.cpp
auto qos = rclcpp::QoS(rclcpp::KeepLast(1)).transient_local().reliable();
map_pub_ = create_publisher<nav_msgs::msg::OccupancyGrid>("/map", qos);

// On activate: publish the map once
map_pub_->on_activate();
map_pub_->publish(build_map());   // DDS caches this for late subscribers
```

**Why TRANSIENT_LOCAL?** The map is static — it does not change during a run. Any node that
subscribes to `/map` after `MapPublisher` has activated should still receive the current map
immediately, without waiting for the next publish cycle. `TRANSIENT_LOCAL` with `KeepLast(1)`
stores exactly the most recent message in the DDS endpoint and replays it to late subscribers.

**Subscriber must also use TRANSIENT_LOCAL:**

```cpp
// From path_planner_server.cpp
map_sub_ = create_subscription<nav_msgs::msg::OccupancyGrid>(
    "/map", rclcpp::QoS(1).transient_local().reliable(),
    [this](nav_msgs::msg::OccupancyGrid::SharedPtr msg) { ... });
```

If `PathPlannerServer` used `VOLATILE`, it would still connect (VOLATILE sub + TRANSIENT_LOCAL
pub = compatible) but would not receive the cached map — it would have to wait for the next
`publish()` call.

## Callback Groups

### The Problem: Single-Threaded Execution

By default, `rclcpp::spin()` uses a `SingleThreadedExecutor`. All callbacks run sequentially
in one thread. If a subscription callback takes 100 ms (e.g., running A* path planning), no
other callbacks run during that time — including timers, service handlers, or other subscriptions.

### MutuallyExclusiveCallbackGroup

A `MutuallyExclusiveCallbackGroup` allows the **callbacks within the group** to be scheduled
by a `MultiThreadedExecutor`, but ensures only **one callback from the group runs at a time**.
This prevents data races within the group while enabling parallelism between groups.

```cpp
// From state_estimator.cpp
auto imu_group  = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
auto odom_group = create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

rclcpp::SubscriptionOptions imu_opts, odom_opts;
imu_opts.callback_group  = imu_group;
odom_opts.callback_group = odom_group;

auto imu_qos = rclcpp::QoS(10).best_effort();
imu_sub_ = create_subscription<custom_interfaces::msg::ImuReading>(
    "/imu", imu_qos,
    std::bind(&StateEstimator::imu_callback, this, std::placeholders::_1),
    imu_opts);   // assigned to imu_group

odom_sub_ = create_subscription<custom_interfaces::msg::Odometry2D>(
    "/odom", 10,
    std::bind(&StateEstimator::odom_callback, this, std::placeholders::_1),
    odom_opts);  // assigned to odom_group
```

**Why two separate MutuallyExclusive groups?** Each subscription is in its own group.
The executor can run `imu_callback` and `odom_callback` **concurrently** because they are in
different groups. Within each group, callbacks are serialized (but there is only one callback
per group, so this has no effect here). The result: the 100 Hz IMU stream is not delayed by
odom processing, and vice versa.

**Why not one ReentrantCallbackGroup?** A `ReentrantCallbackGroup` allows multiple concurrent
invocations of the same callback, which requires all state to be protected by mutexes.
`MutuallyExclusiveCallbackGroup` is safer when the callback accesses shared state (the
`filter_` object) — it guarantees the callback is not re-entered.

### ReentrantCallbackGroup

Use `ReentrantCallbackGroup` when:
- The callback is stateless (pure function of the message)
- You explicitly protect all shared state with mutexes
- Multiple messages may arrive simultaneously and you want maximum throughput

```cpp
// Example — NOT in this showcase, shown for contrast
auto group = create_callback_group(rclcpp::CallbackGroupType::Reentrant);
// Multiple threads can run this callback simultaneously
```

## MultiThreadedExecutor Setup

```cpp
// From state_estimator_main.cpp (conceptual)
int main(int argc, char* argv[]) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<sensor_fusion::StateEstimator>();

    // 2 threads — one per callback group
    rclcpp::executors::MultiThreadedExecutor exec(rclcpp::ExecutorOptions(), 2);
    exec.add_node(node);
    exec.spin();
    rclcpp::shutdown();
    return 0;
}
```

The thread count should match the number of MutuallyExclusiveCallbackGroups that need to run
concurrently. With 2 groups and 2 threads, each group gets a dedicated thread. With only 1
thread, `MultiThreadedExecutor` degrades to `SingleThreadedExecutor` behavior.

For `arm_component_manager.cpp`, all 4 arm nodes share a `MultiThreadedExecutor`:

```cpp
// From arm_component_manager.cpp
auto exec = std::make_shared<rclcpp::executors::MultiThreadedExecutor>();
exec->add_node(js);   // JointStatePublisher
exec->add_node(pid);  // PidController
exec->add_node(ts);   // TrajectoryServer
exec->add_node(gs);   // GripperService
exec->spin();
```

Callbacks from all 4 nodes run in the executor's thread pool, enabling concurrent execution
across nodes within the same process.

## SubscriptionOptions and Callback Group Assignment

The `rclcpp::SubscriptionOptions` struct carries the callback group assignment:

```cpp
rclcpp::SubscriptionOptions opts;
opts.callback_group = my_group;

auto sub = create_subscription<MsgType>(
    "/topic", qos, callback, opts);  // opts is the 4th argument
```

If `opts` is omitted, the subscription joins the default callback group of the node, which
is a `MutuallyExclusiveCallbackGroup` by default. All default-group callbacks are serialized.

## QoS Overrides at Runtime

ROS2 allows QoS settings to be overridden via parameters without changing source code:

```bash
# Override /odom reliability to BEST_EFFORT for this odometry_node instance
ros2 run mobile_robot odometry_node --ros-args \
  -p qos_overrides./odom.publisher.reliability:=best_effort

# Or via launch file
Node(
    package='mobile_robot',
    executable='odometry_node',
    parameters=[{'qos_overrides./odom.publisher.reliability': 'best_effort'}]
)
```

QoS override parameters follow the naming convention:
`qos_overrides.<topic_name>.<endpoint_type>.<policy_name>`.

## Summary: QoS Choices in This Workspace

| Topic | Publisher QoS | Subscriber QoS | Rationale |
|-------|-------------|---------------|-----------|
| `/imu` | BEST_EFFORT | BEST_EFFORT | 100 Hz; dropped packets acceptable for filter |
| `/lidar` | RELIABLE + deadline(150ms) | RELIABLE | Safety-critical; deadline triggers watchdog |
| `/odom` | RELIABLE | RELIABLE | Dead-reckoning; must not lose updates |
| `/fused_odom` | RELIABLE | RELIABLE | Navigation; must not lose fused state |
| `/map` | RELIABLE + TRANSIENT_LOCAL | RELIABLE + TRANSIENT_LOCAL | Static; late joiners need history |
| `/cmd_vel` | RELIABLE | RELIABLE | Velocity commands; loss causes drift |
| `/joint_states` | RELIABLE | RELIABLE | Control loop; every state matters |
