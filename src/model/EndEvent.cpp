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

#include <trace/model/EndEvent.hpp>

namespace trace {

EndEvent::EndEvent(const std::string &uuid, const std::string &name,
                   const EventDefinition &type)
    : Event(uuid, name, type) {}

EndEvent::~EndEvent() {}

Outcome EndEvent::activity() {
  TRACE_LOG_INFO(uuid(), "I promise that I, " + name() + " (" + uuid() +
                             "), am working on it!");

  if (getEventType() != EventDefinition::NONE &&
      getEventType() != EventDefinition::TERMINATION) {
    event_reference_->trigger();
  }
  // deactivate();
  return Outcome();
}

} /* namespace trace */
