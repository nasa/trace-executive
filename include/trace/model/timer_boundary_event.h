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

#ifndef TRACE_MODEL_TIMER_BOUNDARY_EVENT_H_
#define TRACE_MODEL_TIMER_BOUNDARY_EVENT_H_

#include "trace/model/BoundaryEvent.hpp"
#include "trace/model/timer_event_definition.h"

#include <future>
#include <thread>

namespace trace {

/**
 * @brief Timer boundary events can be used to interrupt or branch off an
 * activity whenever the timer expires.
 *
 */
class TimerBoundaryEvent : public BoundaryEvent {

public:
  /**
   * @brief Create a new timer boundary event to add to the model.
   *
   * @param uuid The BPMN assigned universally unique identifier.
   * @param name An optional label for this element.
   * @param definition A definition of this time-based event using ISO8061.
   */
  TimerBoundaryEvent(const std::string &uuid, const std::string &name,
                     const model::TimerEventDefinition &definition);

  TimerBoundaryEvent(const TimerBoundaryEvent &e);

  ~TimerBoundaryEvent();

  COPY_SUPPORT_FUNCTIONS(TimerBoundaryEvent);

  /**
   * @brief A specialized start-up procedure for this event that initiates the
   * time-based event.
   *
   */
  virtual void start() override;

  /**
   * @brief A specialized clean-up procedure for this event that cancels the
   * time-based event if it still active.
   *
   */
  virtual void cleanup() override;

protected:
  /**
   * @brief A specialized activity procedure for this event that waits for the
   * time-based event to transpire.
   *
   * @return Outcome
   */
  virtual Outcome activity() override;

private:
  model::TimerEventDefinition timer_event_definition_;

  std::future<Outcome> timer_future_;
  // std::promise<void> timeout_promise_;

  std::mutex timer_mutex_;
  std::condition_variable timer_interrupted_;
};

} /* namespace trace */

#endif /* PROCESS_ENGINE_INCLUDE_MODEL_ESCALATIONBOUNDARYEVENT_HPP_ */