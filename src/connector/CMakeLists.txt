if(NOT DEFINED PROJECT_CXX_FLAGS)
  set(PROJECT_CXX_FLAGS "-Wall -Wextra -Wpedantic -g -fno-omit-frame-pointer -fPIC -fdiagnostics-color=auto -std=c++17")
endif()

set(CONNECTOR_SOURCES
  Connector.cpp
  ConnectorInterface.cpp
  ConnectorManager.cpp
)

add_library(connector OBJECT ${CONNECTOR_SOURCES})

set_target_properties(
  connector
  PROPERTIES
    COMPILE_FLAGS ${PROJECT_CXX_FLAGS}
)
