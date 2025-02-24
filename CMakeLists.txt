cmake_minimum_required(VERSION 2.8)

project(trace_executive)

set(ABI_VERSION 1)
set(MINOR_VERSION 0.0)

# Set build type
set(CMAKE_BUILD_TYPE Release)

# Set CMAKE_MODULE_PATH for custom CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules ${CMAKE_MODULE_PATH})

# Set compiler flags
set(
  PROJECT_CXX_FLAGS
  "-g -fno-omit-frame-pointer -Wall -Wextra -Werror -fPIC -std=c++11 -fdiagnostics-color=auto"
  # CACHE STRING "Compiler flags for all project targets"
)

message("ABI_VERSION = ${ABI_VERSION}")
message("MINOR_VERSION = ${MINOR_VERSION}")

if("${CATKIN_DEVEL_PREFIX}" STREQUAL "" OR "$ENV{ROS_DISTRO}" STREQUAL "")

else()
  message(STATUS "catkin build was not detected.")

  find_package(catkin REQUIRED COMPONENTS
    roscpp
  )

  catkin_package(
    INCLUDE_DIRS include
    LIBRARIES trace_connector_api trace_executive_api
    CATKIN_DEPENDS roscpp
    # DEPENDS ${Boost_LIBRARIES} dl pthread
  )

endif()


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

# Doxygen
# find_package(Doxygen)
# set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/docs)
# doxygen_add_docs(doxygen
#   WORKINGDIR ${CMAKE_SOURCE_DIR}
#   COMMENT "Generating API documentation with Doxygen")

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


if("${CATKIN_DEVEL_PREFIX}" STREQUAL "" OR "$ENV{ROS_DISTRO}" STREQUAL "")
  message(STATUS "catkin build was not detected.")

  add_executable(trace
    src/main.cpp
  )

  target_link_libraries(trace
    trace_executive_api
    ${Boost_LIBRARIES}
    dl
    pthread
  )

  set_target_properties(
    trace
    PROPERTIES
      COMPILE_FLAGS ${PROJECT_CXX_FLAGS}
  )

  # install

  # When RPM packages are built CMake is invoked with a -DLIB_INSTALL_DIR that we should respect.
  set(
    LIB_INSTALL_DIR lib64
    CACHE STRING "Library install folder name (default: lib64)"
  )

  install(TARGETS trace_connector_api DESTINATION ${LIB_INSTALL_DIR})
  install(TARGETS trace_executive_api DESTINATION ${LIB_INSTALL_DIR})

  install(DIRECTORY ${CMAKE_SOURCE_DIR}/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include
    FILES_MATCHING
    PATTERN "*.hpp"
  )

  install(TARGETS trace DESTINATION bin)

else()
  message(STATUS "catkin build (ros-$ENV{ROS_DISTRO}) was detected.")

  # include_directories(
  #   ${catkin_INCLUDE_DIRS}
  # )

  # add_executable(trace_executive_node
  #   src/RosNodeApplication.cpp
  # )

  # target_link_libraries(trace_executive_node
  #   trace_executive_api
  #   ${Boost_LIBRARIES}
  #   ${catkin_LIBRARIES}
  #   dl
  #   pthread
  # )

  # set_target_properties(trace_executive_node
  #   PROPERTIES
  #     COMPILE_FLAGS ${PROJECT_CXX_FLAGS}
  #     OUTPUT_NAME node
  #     PREFIX ""
  # )

  # Mark executables and/or libraries for installation
  install(
    TARGETS
      trace_connector_api
      trace_executive_api
      # trace_executive_node
    ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
  )

  # Mark cpp header files for installation
  install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION ${CATKIN_GLOBAL_INCLUDE_DESTINATION}
    FILES_MATCHING
    PATTERN "*.hpp"
    PATTERN "*.h"
  )

endif()
