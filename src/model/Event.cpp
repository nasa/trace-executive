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

#include <trace/model/Event.hpp>

// std
#include <stdexcept>

namespace trace {

Event::Event(const std::string &uuid, const std::string &name,
             const EventDefinition type)
    : ProcessModelNode(uuid, name), event_type_(type) {}

Event::Event(const Event &event) : ProcessModelNode(event) {
  event_reference_ = event.event_reference_;
  event_type_ = event.event_type_;
}

std::string Event::to_string(EventDefinition e) {
  switch (e) {
  case EventDefinition::NONE:
    return "NONE";
  case EventDefinition::MESSAGE:
    return "MESSAGE";
  case EventDefinition::TIMER:
    return "TIMER";
  case EventDefinition::SIGNAL:
    return "SIGNAL";
  case EventDefinition::ESCALATION:
    return "ESCALATION";
  case EventDefinition::TERMINATION:
    return "TERMINATION";
  case EventDefinition::ERROR:
    return "ERROR";
  default:
    return "UNDEFINED";
  }
}

void Event::attachEventReference(std::shared_ptr<ModelEvent> event_reference) {
  TRACE_LOG_INFO(uuid(), "event_type_ (" << to_string(event_type_) << ")");
  TRACE_LOG_INFO(uuid(), "event_reference_type_ ("
                             << to_string(event_reference->getEventType())
                             << ")");
  if (event_reference->getEventType() == event_type_) {
    event_reference_ = event_reference;
  } else {
    throw std::runtime_error(
        "Cannot attach an event reference of a different type.");
  }
}

boost::signals2::connection Event::attachToEvent(
    boost::function<void(const std::shared_ptr<ModelEvent> &)> callback) {
  return event_reference_->attachEventResultCallback(callback);
}

EventDefinition Event::getEventType() {
  if (event_reference_ != nullptr) {
    return event_reference_->getEventType();
  } else {
    return event_type_;
  }
}

std::string Event::getEventUuid() { return event_reference_->getEventUuid(); }

std::string Event::getEventName() { return event_reference_->getEventName(); }

void Event::trigger() { event_reference_->trigger(); }

} /* namespace trace */
