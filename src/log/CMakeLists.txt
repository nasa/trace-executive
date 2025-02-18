if(NOT DEFINED PROJECT_CXX_FLAGS)
  set(PROJECT_CXX_FLAGS "-Wall -Wextra -Wpedantic -g -fno-omit-frame-pointer -fPIC -fdiagnostics-color=auto -std=c++17")
endif()

set(LOG_SOURCES
  connector_output_target.cpp
  location.cpp
  logger.cpp
  message.cpp
  severity_level.cpp
  standard_output_target.cpp
  timestamp.cpp
)

add_library(log OBJECT ${LOG_SOURCES})

set_target_properties(
  log
  PROPERTIES
    COMPILE_FLAGS ${PROJECT_CXX_FLAGS}
)
