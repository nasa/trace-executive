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

#include <trace/model/StartEvent.hpp>

// boost
#include <boost/bind/bind.hpp>

namespace trace {

StartEvent::StartEvent(const std::string &uuid, const std::string &name,
                       const EventDefinition &type)
    : Event(uuid, name, type) {}

StartEvent::~StartEvent() {}

void StartEvent::start() {
  if (getEventType() != EventDefinition::NONE) {
    signal_connection_ =
        event_reference_->attachEventResultCallback(boost::bind(
            &StartEvent::event_callback, this, boost::placeholders::_1));
  }
  Event::start();
}

Outcome StartEvent::activity() {
  TRACE_LOG_INFO(uuid(), "I promise that I, " + name() + " (" + uuid() +
                             "), am working on it!");

  if (event_type_ == EventDefinition::NONE) {
    return Outcome(StatusCode::OK, "Nothing to catch here!");
  }

  bool event_caught = false;
  std::unique_lock<std::mutex> lock(is_active_mutex_);
  external_termination_signaled_.wait(lock);
  event_caught = event_caught_;
  lock.unlock();

  if (event_caught) {
    return Outcome(StatusCode::OK, "Received event notification.");
  } else {
    return Outcome(StatusCode::CANCELLED,
                   "Cancelled before event was signaled!");
  }
}

void StartEvent::cleanup() { signal_connection_.disconnect(); }

void StartEvent::event_callback(const std::shared_ptr<ModelEvent> &event) {
  {
    std::lock_guard<std::mutex> lock(is_active_mutex_);
    event_caught_ = true;
  }
  external_termination_signaled_.notify_all();
  TRACE_LOG_INFO(
      uuid(),
      "Caught event ("
          << event->getEventUuid()
          << ")."); //" of type (" <<
                    // std::to_string(event->getEventType()) << ").");
}

} /* namespace trace */
