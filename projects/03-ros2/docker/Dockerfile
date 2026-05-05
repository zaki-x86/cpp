FROM ros:humble-ros-base-jammy

RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y --no-install-recommends \
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-pip \
    ros-humble-rviz2 \
    ros-humble-rclcpp-components \
    ros-humble-tf2-ros \
    ros-humble-tf2-geometry-msgs \
    ros-humble-nav-msgs \
    ros-humble-sensor-msgs \
    ros-humble-diagnostic-msgs \
    ros-humble-geometry-msgs \
    ros-humble-launch-testing-ament-cmake \
    ros-humble-ament-cmake-gtest \
    x11-apps \
    mesa-utils \
    libgl1 \
    && rm -rf /var/lib/apt/lists/*

# Initialize rosdep
RUN rosdep init || true && rosdep update --rosdistro humble

WORKDIR /ros2_ws

COPY docker/entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
CMD ["bash"]
