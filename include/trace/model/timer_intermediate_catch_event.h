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

#ifndef TRACE_MODEL_TIMER_INTERMEDIATE_CATCH_EVENT_H_
#define TRACE_MODEL_TIMER_INTERMEDIATE_CATCH_EVENT_H_

#include "trace/model/IntermediateCatchEvent.hpp"
#include "trace/model/timer_event_definition.h"

#include <condition_variable>
#include <future>
#include <mutex>

namespace trace {

class TimerIntermediateCatchEvent : public IntermediateCatchEvent {

public:
  TimerIntermediateCatchEvent(const std::string &uuid, const std::string &name,
                              const model::TimerEventDefinition &definition);
  TimerIntermediateCatchEvent(
      const TimerIntermediateCatchEvent &timer_intermediate_catch_event);
  virtual ~TimerIntermediateCatchEvent();

  COPY_SUPPORT_FUNCTIONS(TimerIntermediateCatchEvent);

protected:
  virtual Outcome activity();

private:
  model::TimerEventDefinition timer_event_definition_;
  std::promise<void> timeout_promise_;
};

} /* namespace trace */

#endif /* TRACE_MODEL_TIMER_INTERMEDIATE_CATCH_EVENT_H_ */
