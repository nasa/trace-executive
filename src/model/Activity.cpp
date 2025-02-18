/**
 * Copyright 2016-2025 California Institute of Technology
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "trace/model.h"

#include <trace/model/Activity.hpp>
#include <trace/model/SequenceFlow.hpp>

namespace trace {

Activity::Activity(const std::string &uuid, const std::string &name)
    : ProcessModelNode(uuid, name), is_interrupted_(false) {
  TRACE_LOG_ERROR(uuid, "This is activity " << name << " with no events");
}

Activity::Activity(const Activity &reference) : ProcessModelNode(reference) {
  boundary_events_ = reference.boundary_events_;

  TRACE_LOG_ERROR(uuid(), "This is activity " << reference.name() << " with "
                                              << boundary_events_.size()
                                              << " events");
  is_interrupted_ = false;
}

Activity::~Activity() {}

void Activity::addBoundaryEvent(
    const std::string &uuid, std::shared_ptr<BoundaryEvent> &boundary_event) {
  TRACE_LOG_ERROR(this->uuid(), "This is activity " << this->uuid() << ", "
                                                    << this->name()
                                                    << " adding new BE ");

  boundary_events_[uuid] = boundary_event;
}

std::unordered_map<std::string, std::shared_ptr<BoundaryEvent>>
Activity::listBoundaryEvents() {
  return boundary_events_;
}

void Activity::interrupt() {
  TRACE_LOG_INFO(uuid(), "calling base interrupt()");
  is_interrupted_ = true;
  terminate();
}

void Activity::boundary_event_callback(
    const std::shared_ptr<ModelEvent> &event) {

  TRACE_LOG_INFO(
      uuid(), "Activity [" << uuid() << "] received notification of an event.");

  TRACE_LOG_INFO(uuid(), "[" << uuid() << "] has "
                             << listBoundaryEvents().size()
                             << " boundary events.");
  for (auto kv_pair : listBoundaryEvents()) {
    TRACE_LOG_INFO(uuid(), "Boundary event lists ["
                               << kv_pair.second->getEventUuid() << "]");
    TRACE_LOG_INFO(uuid(), "Event notification lists [" << event->getEventUuid()
                                                        << "].");
    if (kv_pair.second->getEventUuid().compare(event->getEventUuid()) == 0) {
      TRACE_LOG_INFO(uuid(), "Matching boundary event found.");

      // Second, terminate this activity if the boundary event is interrupting
      if (kv_pair.second->getCancelActivity()) {
        {
          std::lock_guard<std::mutex> lock(is_active_mutex_);
          interrupting_boundary_event_uuid_ = kv_pair.first;
        }
        interrupt();
      } else {
        // invoke a new token to handle the boundary event outbounds
        // auto parent_uuid = this->instance_->getParentProcessUuid(uuid_);
        for (auto kv_uuid : kv_pair.second->listOutputPorts()) {
          if (kv_uuid.second->evaluateExpression()) {
            TRACE_LOG_INFO(uuid(),
                           "Spawning new token at ["
                               << kv_uuid.first
                               << "] as a follow-on to this boundary event.");
            this->instance_->emitNewToken(kv_uuid.first);
          }
        }
      }
    }
  }
}

void Activity::resume() {
  activateBoundaryEvents();
  TRACE_LOG_INFO(uuid(), "Custom resume() called.");
  ProcessModelNode::resume();
}

void Activity::start() {
  activateBoundaryEvents();
  TRACE_LOG_INFO(uuid(), "Custom start() called.");
  ProcessModelNode::start();
}

void Activity::cleanup() {
  for (auto connection : boundary_event_connections_) {
    connection.disconnect();
  }
  boundary_event_connections_.clear();

  for (auto kv_pair : listBoundaryEvents()) {
    kv_pair.second->cleanup();
  }

  TRACE_LOG_INFO(uuid(), "Custom cleanup() called.");
  ProcessModelNode::cleanup();
}

void Activity::activateBoundaryEvents() {
  TRACE_LOG_INFO(uuid(), "This activity has ("
                             << listBoundaryEvents().size()
                             << ") boundary event(s) attached.");
  for (auto kv_pair : listBoundaryEvents()) {
    TRACE_LOG_INFO(uuid(), "Activating boundary event ("
                               << kv_pair.second->uuid() << ", "
                               << kv_pair.second->name() << ").");
    boundary_event_connections_.push_back(kv_pair.second->attachToEvent(
        boost::bind(&Activity::boundary_event_callback, this,
                    boost::placeholders::_1)));
    kv_pair.second->start();
  }
}

} /* namespace trace */
