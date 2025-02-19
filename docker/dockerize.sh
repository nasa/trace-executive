#!/bin/bash

export DOCKER_BUILDKIT=1

docker build \
    --no-cache \
    --ssh default \
    -t trace-ros2:latest .