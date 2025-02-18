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

#include "trace/model/timer_boundary_event.h"

namespace trace {

TimerBoundaryEvent::TimerBoundaryEvent(
    const std::string &uuid, const std::string &name,
    const model::TimerEventDefinition &definition)
    : BoundaryEvent(uuid, name, EventDefinition::TIMER),
      timer_event_definition_(definition) {}

TimerBoundaryEvent::TimerBoundaryEvent(const TimerBoundaryEvent &e)
    : BoundaryEvent(e), timer_event_definition_(e.timer_event_definition_) {}

TimerBoundaryEvent::~TimerBoundaryEvent() {}

void TimerBoundaryEvent::start() {
  // no-op
  TRACE_LOG_INFO(uuid(), "Start by launching the timer loop.");
  this->timer_future_ =
      std::async(std::launch::async, &TimerBoundaryEvent::activity, this);
  TRACE_LOG_INFO(uuid(), "Timer loop was launched. Done with start.");
}

Outcome TimerBoundaryEvent::activity() {
  // this->timeout_promise_ = std::promise<void>();
  // auto timer_interrupt = this->timeout_promise_.get_future();
  TRACE_LOG_INFO(uuid(), "Waiting on the timer to expire...");
  std::unique_lock<std::mutex> lock(this->timer_mutex_);
  bool is_timeout = this->timer_event_definition_.WaitOn(
      this->timer_interrupted_, lock); //(timer_interrupt);
  if (is_timeout) {
    TRACE_LOG_INFO(uuid(), "...expired");
    trigger();
  }
  return Outcome(StatusCode::OK, "");
}

void TimerBoundaryEvent::cleanup() {
  // no-op
  TRACE_LOG_INFO(uuid(),
                  "Stop by interrupting the timer loop if it is still active.");
  // this->timeout_promise_.set_value();
  this->timer_interrupted_.notify_all();
  if (this->timer_future_.valid()) {
    timer_future_.wait();
  }
  TRACE_LOG_INFO(uuid(), "Done cleaning up this boundary event.");
}

} // namespace trace
