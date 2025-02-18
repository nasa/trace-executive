cmake_minimum_required(VERSION 3.16)
project(trace_executive)

# Use C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

# Compiler flags
add_compile_options(-Wall -Wextra -Wpedantic)

# Set build type if not explicitly defined
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Set CMAKE_MODULE_PATH for custom CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules ${CMAKE_MODULE_PATH})

set(
  PROJECT_CXX_FLAGS
  "-g -fno-omit-frame-pointer -Wall -Wextra -Werror -fPIC -std=c++14 -fdiagnostics-color=auto"
)

# Versioning
set(ABI_VERSION 1)
set(MINOR_VERSION 0.0)
message("ABI_VERSION = ${ABI_VERSION}")
message("MINOR_VERSION = ${MINOR_VERSION}")

message(STATUS "Building within a ROS $ENV{ROS_VERSION} environment.")

# Find ROS 2 dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)

# Find system dependencies

# Boost 1.53

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(Boost_MIN_VERSION "1.53.0")

find_package(Boost REQUIRED COMPONENTS date_time program_options regex serialization)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIRS}
)


add_subdirectory(src)

## trace_connector_api

add_library(trace_connector_api SHARED
  src/connector/Connector.cpp
  src/connector/ConnectorInterface.cpp
  src/Outcome.cpp
  src/types/SeverityLevel.cpp
)

set_target_properties(trace_connector_api
  PROPERTIES
    COMPILE_FLAGS ${PROJECT_CXX_FLAGS}
    SOVERSION ${ABI_VERSION}
    VERSION ${ABI_VERSION}.${MINOR_VERSION}
)

## trace_executive_api

add_library(trace_executive_api SHARED
  $<TARGET_OBJECTS:bpmn>
  $<TARGET_OBJECTS:connector>
  $<TARGET_OBJECTS:control>
  $<TARGET_OBJECTS:core>
  $<TARGET_OBJECTS:log>
  $<TARGET_OBJECTS:model>
  $<TARGET_OBJECTS:model_extension>
  $<TARGET_OBJECTS:storage>
  $<TARGET_OBJECTS:types>
)

target_link_libraries(trace_executive_api
  ${Boost_LIBRARIES}
  dl
)

set_target_properties(
  trace_executive_api
  PROPERTIES
    COMPILE_FLAGS ${PROJECT_CXX_FLAGS}
    SOVERSION ${ABI_VERSION}
    VERSION ${ABI_VERSION}.${MINOR_VERSION}
)

if(DEFINED DOCTEST_CONFIG_DISABLE)
  enable_testing()
  add_subdirectory(test)
endif()

message(STATUS "ament build (ros-$ENV{ROS_DISTRO}) was detected.")

# Mark executables and/or libraries for installation
install(
  TARGETS
    trace_connector_api
    trace_executive_api
  EXPORT trace_apis
  ARCHIVE DESTINATION lib 
  LIBRARY DESTINATION lib 
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

# Mark cpp header files for installation
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
  DESTINATION lib/${PROJECT_NAME}
  FILES_MATCHING
  PATTERN "*.hpp"
  PATTERN "*.h"
)

install(DIRECTORY include/
  DESTINATION include
)

ament_export_include_directories(include)
ament_export_targets(trace_apis HAS_LIBRARY_TARGET)

ament_package()