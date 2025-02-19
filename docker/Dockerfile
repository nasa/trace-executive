ARG FROM_IMAGE=arm64v8/ros:humble-ros-base-jammy
ARG OVERLAY_WS=/opt/ros/overlay_ws

# multi-stage for caching
FROM $FROM_IMAGE AS cacher

# clone overlay source
ARG OVERLAY_WS
WORKDIR $OVERLAY_WS/src
COPY trace.repos ../trace.repos

RUN apt-get update && apt-get install -y --no-install-recommends \
    python3-pip git openssh-client
RUN pip install -U vcstool

RUN mkdir -p -m 0600 ~/.ssh && ssh-keyscan github-fn.jpl.nasa.gov >> ~/.ssh/known_hosts
RUN --mount=type=ssh vcs import ./ < ../trace.repos

# copy manifests for caching
WORKDIR /opt
RUN mkdir -p /tmp/opt && \
    find ./ -name "package.xml" | \
      xargs cp --parents -t /tmp/opt && \
    find ./ -name "CATKIN_IGNORE" | \
      xargs cp --parents -t /tmp/opt || true

# multi-stage for building
FROM $FROM_IMAGE AS builder

# install overlay dependencies
ARG OVERLAY_WS
WORKDIR $OVERLAY_WS
COPY --from=cacher /tmp/$OVERLAY_WS/src ./src
RUN . /opt/ros/$ROS_DISTRO/setup.sh && \
    apt-get update && rosdep install -y \
      --from-paths \
        src/trace_executive \
        src/trace_ros_executive \
      --ignore-src \
    && rm -rf /var/lib/apt/lists/*

# build overlay source
COPY --from=cacher $OVERLAY_WS/src ./src
RUN . /opt/ros/$ROS_DISTRO/setup.sh && \
    colcon build

# run launch file
CMD ["bash", "-c", "source install/setup.bash && ros2 launch trace_example_ros_executive example.launch"]