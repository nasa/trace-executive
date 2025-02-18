cmake_minimum_required(VERSION 3.5)

if(NOT DEFINED PROJECT_CXX_FLAGS)
  set(PROJECT_CXX_FLAGS "-Wall -Wextra -Wpedantic -g -fno-omit-frame-pointer -fPIC -fdiagnostics-color=auto -std=c++17")
endif()

add_subdirectory(bpmn)
add_subdirectory(connector)
add_subdirectory(control)
add_subdirectory(log)
add_subdirectory(model)
add_subdirectory(storage)
add_subdirectory(types)

set(CORE_SOURCES
  model.cpp
  ModelEvent.cpp
  Object.cpp
  Outcome.cpp
  Snapshot.cpp
  SnapshotProcessor.cpp
  # timeline.cpp
  token.cpp
)

add_library(core OBJECT ${CORE_SOURCES})

target_include_directories(core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include> 
  $<INSTALL_INTERFACE:include>
)


set_target_properties(
  core
  PROPERTIES
    COMPILE_FLAGS ${PROJECT_CXX_FLAGS}
)

