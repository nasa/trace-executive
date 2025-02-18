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

#ifndef TRACE_MODEL_STARTEVENT_HPP_
#define TRACE_MODEL_STARTEVENT_HPP_

#include <trace/ModelEvent.hpp>
#include <trace/model/Event.hpp>

#include <boost/signals2/connection.hpp>

#include <condition_variable>
#include <mutex>

namespace trace {

class StartEvent : public Event {
public:
  StartEvent(const std::string &uuid, const std::string &name,
             const EventDefinition &type = EventDefinition::NONE);
  virtual ~StartEvent();

  COPY_SUPPORT_FUNCTIONS(StartEvent);

  void event_callback(const std::shared_ptr<ModelEvent> &event);

protected:
  virtual void start();
  virtual Outcome activity();
  virtual void cleanup();

  boost::signals2::connection signal_connection_;

  bool event_caught_ = false;
};

} /* namespace trace */

#endif /* TRACE_MODEL_STARTEVENT_HPP_ */
