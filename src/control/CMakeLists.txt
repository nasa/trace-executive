if(NOT DEFINED PROJECT_CXX_FLAGS)
  set(PROJECT_CXX_FLAGS "-Wall -Wextra -Wpedantic -g -fno-omit-frame-pointer -fPIC -fdiagnostics-color=auto -std=c++17")
endif()

set(CONTROL_SOURCES
  mission_control.cpp
)

add_library(control OBJECT ${CONTROL_SOURCES})

set_target_properties(
  control
  PROPERTIES
    COMPILE_FLAGS ${PROJECT_CXX_FLAGS}
)
