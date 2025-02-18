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

#include "trace/model/timer_intermediate_catch_event.h"

namespace trace {

TimerIntermediateCatchEvent::TimerIntermediateCatchEvent(
    const std::string &uuid, const std::string &name,
    const model::TimerEventDefinition &definition)
    : IntermediateCatchEvent(uuid, name, EventDefinition::TIMER),
      timer_event_definition_(definition) {}

TimerIntermediateCatchEvent::TimerIntermediateCatchEvent(
    const TimerIntermediateCatchEvent &timer_intermediate_catch_event)
    : IntermediateCatchEvent(timer_intermediate_catch_event),
      timer_event_definition_(
          timer_intermediate_catch_event.timer_event_definition_) {}

TimerIntermediateCatchEvent::~TimerIntermediateCatchEvent() {}

Outcome TimerIntermediateCatchEvent::activity() {

  // TRACE_LOG_INFO(uuid(), "I promise that I, " + name() + " (" + uuid() +
  //             "), will sleep for " + timer_duration_msec_ + "ms!");

  TRACE_LOG_INFO(uuid(), "I promise that I, " + name() + " (" + uuid() +
            "), will sleep for until it is time!");

  TRACE_LOG_INFO(uuid(), "Waiting on the timer to expire...");

  {
    std::unique_lock<std::mutex> lock(this->is_active_mutex_);
    const auto is_timeout = this->timer_event_definition_.WaitOn(
        external_termination_signaled_, lock);
    if (is_timeout) {
      TRACE_LOG_INFO(uuid(), "...expired");
      return Outcome(StatusCode::OK, "Timer expired!");
    } else {
      return Outcome(StatusCode::CANCELLED,
                     "Timer was deactivated before it expired.");
    }
  }
}

} /* namespace trace */
