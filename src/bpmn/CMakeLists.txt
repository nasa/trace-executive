if(NOT DEFINED PROJECT_CXX_FLAGS)
  set(PROJECT_CXX_FLAGS "-Wall -Wextra -Wpedantic -g -fno-omit-frame-pointer -fPIC -fdiagnostics-color=auto -std=c++17")
endif()

set(BPMN_SOURCES
  EventNotations.cpp
  Parser.cpp
)

add_library(bpmn OBJECT ${BPMN_SOURCES})

set_target_properties(
  bpmn
  PROPERTIES
    COMPILE_FLAGS ${PROJECT_CXX_FLAGS}
)
