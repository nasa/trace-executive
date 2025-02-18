if(NOT DEFINED PROJECT_CXX_FLAGS)
  set(PROJECT_CXX_FLAGS "-Wall -Wextra -Wpedantic -g -fno-omit-frame-pointer -fPIC -fdiagnostics-color=auto -std=c++17")
endif()

set(TYPES_SOURCES
  SeverityLevel.cpp
)

add_library(types OBJECT ${TYPES_SOURCES})

set_target_properties(
  types
  PROPERTIES
    COMPILE_FLAGS ${PROJECT_CXX_FLAGS}
)
