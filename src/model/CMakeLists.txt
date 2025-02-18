add_subdirectory(extension)

set(MODEL_SOURCES
  Activity.cpp
  BoundaryEvent.cpp
  CallActivity.cpp
  EndEvent.cpp
  ErrorBoundaryEvent.cpp
  EscalationBoundaryEvent.cpp
  EscalationEndEvent.cpp
  EscalationIntermediateCatchEvent.cpp
  EscalationIntermediateThrowEvent.cpp
  Event.cpp
  ExclusiveGateway.cpp
  Flow.cpp
  flow_element.cc
  Gateway.cpp
  InclusiveGateway.cpp
  IntermediateCatchEvent.cpp
  IntermediateThrowEvent.cpp
  MessageBoundaryEvent.cpp
  MessageEndEvent.cpp
  MessageIntermediateCatchEvent.cpp
  MessageIntermediateThrowEvent.cpp
  MessageStartEvent.cpp
  ParallelGateway.cpp
  Process.cpp
  ProcessModelNode.cpp
  ScriptTask.cpp
  SequenceFlow.cpp
  ServiceTask.cpp
  SignalBoundaryEvent.cpp
  SignalEndEvent.cpp
  SignalIntermediateCatchEvent.cpp
  SignalIntermediateThrowEvent.cpp
  SignalStartEvent.cpp
  StartEvent.cpp
  Task.cpp
  TerminationEndEvent.cpp
  timer_boundary_event.cc
  timer_event_definition.cc
  timer_intermediate_catch_event.cc
  UserTask.cpp
)

message("MODEL_SOURCES = ${MODEL_SOURCES}")

add_library(model OBJECT ${MODEL_SOURCES})

set_target_properties(
  model
  PROPERTIES
    COMPILE_FLAGS ${PROJECT_CXX_FLAGS}
)
