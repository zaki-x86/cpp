# Docker Setup

This document explains the Docker infrastructure for the ROS2 showcase, covering the
Dockerfile, entrypoint, Compose services, WSL2 X11 forwarding, and all operational commands.

## Dockerfile Walkthrough

```dockerfile
FROM ros:humble-ros-base-jammy
```

`ros:humble-ros-base-jammy` is the minimal ROS2 Humble image built on Ubuntu 22.04 LTS
(Jammy). It includes `rclcpp`, `ament_cmake`, and the basic DDS middleware (FastDDS), but
**no GUI tools**. This choice keeps the image lean (~800 MB vs ~3.5 GB for
`ros:humble-desktop`). RViz2 is added explicitly as a separate apt package so the dev service
can build without it if desired. Using `ros-base` also avoids shipping a full X11 stack in
the build environment.

### APT Packages and Their Purposes

```dockerfile
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y --no-install-recommends \
    python3-colcon-common-extensions \   # colcon build/test/test-result commands
    python3-rosdep \                     # dependency resolver for ROS packages
    python3-pip \                        # pip (used for cmake upgrade if needed)
    ros-humble-rviz2 \                  # 3D visualization; only needed for rviz2 service
    ros-humble-rclcpp-components \      # composable node loader, register_node_macro
    ros-humble-tf2-ros \                # TransformBroadcaster, StaticTransformBroadcaster
    ros-humble-tf2-geometry-msgs \      # quaternion / vector conversion utilities
    ros-humble-nav-msgs \               # OccupancyGrid, Odometry
    ros-humble-sensor-msgs \            # JointState, LaserScan
    ros-humble-diagnostic-msgs \        # DiagnosticStatus, DiagnosticArray
    ros-humble-geometry-msgs \          # Twist, TransformStamped, Vector3
    ros-humble-launch-testing-ament-cmake \  # ament_add_gtest integration
    ros-humble-ament-cmake-gtest \          # ament_cmake_gtest macro
    x11-apps \                          # xeyes, xclock — X11 smoke-test tools
    mesa-utils \                        # glxinfo — OpenGL diagnostics for RViz2
    libgl1 \                            # OpenGL shared library for RViz2 rendering
    && rm -rf /var/lib/apt/lists/*      # clear apt cache — reduces layer size by ~60 MB
```

### rosdep Initialization

```dockerfile
RUN rosdep init || true && rosdep update --rosdistro humble
```

`rosdep init` writes `/etc/ros/rosdep/sources.list.d/20-default.list`. The `|| true` guard
prevents a build failure if the file already exists (common in CI that reuses layers).
`rosdep update` pulls the package key database from GitHub so `rosdep install` can resolve
system dependencies from `package.xml` declarations.

### WORKDIR and ENTRYPOINT

```dockerfile
WORKDIR /ros2_ws
COPY docker/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
```

`WORKDIR /ros2_ws` ensures every `docker compose run` command starts in the workspace root.
`ENTRYPOINT` (exec form) ensures the script runs as PID 1's immediate child, preserving
correct signal propagation (SIGTERM → colcon/ros2 process, not bash).

## entrypoint.sh Explanation

```bash
#!/bin/bash
set -e

# Source ROS2 Humble
source /opt/ros/humble/setup.bash

# Source workspace if built
if [ -f /ros2_ws/install/setup.bash ]; then
    source /ros2_ws/install/setup.bash
fi

exec "$@"
```

**Why source both setup files?**

- `/opt/ros/humble/setup.bash` sets `AMENT_PREFIX_PATH`, `LD_LIBRARY_PATH`, and `PATH`
  for all ROS2 Humble system packages. Without this, `ros2` command is not found.
- `/ros2_ws/install/setup.bash` overlays the workspace packages (custom_interfaces,
  mobile_robot, etc.) on top of the system install. It is optional because on a clean
  container the workspace has not been built yet — hence the `if` guard.

**Why `exec "$@"`?**

`exec` replaces the shell process with the command passed as Docker CMD or
`docker compose run` arguments. This means the subprocess (e.g., `colcon build`, `ros2 launch`)
becomes PID 1 in the container and receives Docker stop signals (SIGTERM) directly.
Without `exec`, bash catches the signal and the child may be left running for the full 10 s
Docker stop grace period before SIGKILL.

## docker-compose.yml Deep Dive

### `dev` Service

```yaml
dev:
  build:
    context: .
    dockerfile: docker/Dockerfile
  image: ros2_showcase:humble
  volumes:
    - ./ros2_ws:/ros2_ws      # persistent — colcon build output survives container exit
  environment:
    - ROS_DOMAIN_ID=42        # DDS domain isolation
  network_mode: host          # DDS multicast discovery without port mapping
  stdin_open: true
  tty: true
  command: bash
```

**`network_mode: host`**: ROS2 uses DDS (FastDDS) for discovery. DDS relies on UDP multicast
(`239.255.0.1:7400` for participant discovery). With Docker's default bridge network, multicast
packets are not forwarded between the host and container, so `ros2 topic list` on the host
cannot see nodes running in the container. `host` networking removes the network namespace
entirely — the container shares the host's network interfaces, so DDS discovery works without
any port mapping configuration.

**`./ros2_ws:/ros2_ws` volume**: `colcon build` writes compiled libraries to
`/ros2_ws/install/`, build artifacts to `/ros2_ws/build/`. By mounting the workspace from the
host, these outputs persist across `docker compose run --rm` invocations. The `--rm` flag
removes the container but not the volume data. Subsequent builds are incremental.

**`ROS_DOMAIN_ID=42`**: ROS2 nodes on the same machine communicate if they share a Domain ID.
Setting a non-default value (default is 0) isolates this workspace from any other ROS2
processes on the host or CI runners that use domain 0.

### `rviz2` Service

```yaml
rviz2:
  image: ros2_showcase:humble
  volumes:
    - ./ros2_ws:/ros2_ws
    - /tmp/.X11-unix:/tmp/.X11-unix:rw   # X11 Unix socket
  environment:
    - DISPLAY=${DISPLAY}                  # X server display number from host
    - ROS_DOMAIN_ID=42
  network_mode: host
  command: bash -c "if [ -f /ros2_ws/src/mobile_robot/rviz/full_demo.rviz ]; then
                      rviz2 -d /ros2_ws/src/mobile_robot/rviz/full_demo.rviz;
                    else rviz2; fi"
```

RViz2 uses the X11 protocol to render on the host display. The `/tmp/.X11-unix` socket is the
Unix domain socket that X clients use to connect to the Xorg/XWayland server on the host.
`DISPLAY=:0` tells the X client library which server to connect to (display 0 on the local
machine). The socket mount plus `DISPLAY` are sufficient — no SSH tunneling required for WSL2.

## WSL2 X11 Forwarding Setup

WSL2 runs a Linux kernel in a Hyper-V VM. The Windows host runs an X server (VcXsrv, Xming,
or the built-in WSLg). For Docker containers running inside WSL2 to display windows on the
Windows desktop:

```bash
# Step 1 — Set display (run once per WSL2 session)
export DISPLAY=:0

# Step 2 — Grant Docker containers access to the X server
xhost +local:docker

# Step 3 — Verify X11 is working from WSL2 itself
xeyes   # should open a window on the Windows desktop
```

`xhost +local:docker` grants access to any local process (including Docker containers
using host networking). This is safe on a developer workstation; for shared machines prefer
`xhost +si:localuser:$(whoami)`.

**WSLg users** (Windows 11 + WSL2 2.x): WSLg provides a built-in Wayland/X11 compositor.
The socket at `/tmp/.X11-unix/X0` is already present. `DISPLAY=:0` works without installing
a separate X server.

## All Commands Reference

### Build the Docker image

```bash
cd projects/03-ros2
docker compose build
```

Builds `ros2_showcase:humble` from `docker/Dockerfile`. Takes ~3 minutes on first run;
subsequent runs use layer cache for the apt install step.

### Build the ROS2 workspace

```bash
docker compose run --rm dev bash -c "cd /ros2_ws && colcon build --symlink-install"
```

`--symlink-install` creates symlinks for Python files and resource files instead of copying,
so source edits are reflected without rebuilding. C++ code still requires a rebuild.
Expected output: `Summary: 4 packages finished`.

### Run all tests

```bash
docker compose run --rm dev bash -c "cd /ros2_ws && colcon test --event-handlers console_direct+ && colcon test-result --verbose"
```

Expected: 18 tests, 0 errors, 0 failures. Packages tested: `arm_controller` (5 PID tests),
`mobile_robot` (5 A* tests), `sensor_fusion` (5 complementary filter tests).

### Run mobile_robot demo

```bash
docker compose run --rm dev ros2 launch mobile_robot mobile_robot.launch.py
```

Starts: map_publisher, odometry_node, velocity_controller, path_planner_server.
Lifecycle nodes auto-configure and auto-activate via `TimerAction` + `EmitEvent`.

### Run full demo (all packages)

```bash
docker compose run --rm dev ros2 launch mobile_robot full_demo.launch.py
```

Adds sensor_fusion nodes (imu_simulator, lidar_simulator, state_estimator,
diagnostics_aggregator) on top of the mobile_robot demo.

### Open RViz2

```bash
# On WSL2 host first:
export DISPLAY=:0
xhost +local:docker

# In Docker:
docker compose run --rm rviz2
```

### Introspect the running system

```bash
# In a separate terminal while nodes are running:
docker compose run --rm dev bash -c "source /ros2_ws/install/setup.bash && ros2 node list"
docker compose run --rm dev bash -c "source /ros2_ws/install/setup.bash && ros2 topic list"
docker compose run --rm dev bash -c "source /ros2_ws/install/setup.bash && ros2 service list"
docker compose run --rm dev bash -c "source /ros2_ws/install/setup.bash && ros2 action list"
```

## ROS_DOMAIN_ID Explanation

DDS (Data Distribution Service) organizes participants into numbered domains (0–232).
Participants on different domain IDs cannot communicate. Setting `ROS_DOMAIN_ID=42`:

- Prevents this workspace from interfering with ROS2 tutorials or other projects on the same
  machine (which commonly use domain 0)
- Allows running multiple workspaces simultaneously on different domains
- Affects all nodes in both `dev` and `rviz2` services (set in environment section)

To talk to these nodes from the host terminal: `export ROS_DOMAIN_ID=42` before any `ros2` command.
