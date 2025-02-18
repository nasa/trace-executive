if(NOT DEFINED PROJECT_CXX_FLAGS)
  set(PROJECT_CXX_FLAGS "-default-flags-here")
endif()

set(MODEL_EXTENSION_SOURCES
  ConnectorServiceTask.cpp
  ConnectorUserTask.cpp
)

message("MODEL_EXTENSION_SOURCES = ${MODEL_EXTENSION_SOURCES}")

add_library(model_extension OBJECT ${MODEL_EXTENSION_SOURCES})

set_target_properties(
  model_extension
  PROPERTIES
    COMPILE_FLAGS ${PROJECT_CXX_FLAGS}
)
