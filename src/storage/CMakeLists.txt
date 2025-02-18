if(NOT DEFINED PROJECT_CXX_FLAGS)
  set(PROJECT_CXX_FLAGS "-Wall -Wextra -Wpedantic -g -fno-omit-frame-pointer -fPIC -fdiagnostics-color=auto -std=c++17")
endif()

set(STORAGE_SOURCES
  DataStore.cpp
)

add_library(storage OBJECT ${STORAGE_SOURCES})

set_target_properties(
  storage
  PROPERTIES
    COMPILE_FLAGS ${PROJECT_CXX_FLAGS}
)
