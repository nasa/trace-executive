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

#ifndef TRACE_MODEL_EVENT_HPP_
#define TRACE_MODEL_EVENT_HPP_

#include <trace/model/ProcessModelNode.hpp>

// std
#include <memory>

// local
#include <trace/ModelEvent.hpp>
#include <trace/Types.hpp>

namespace trace {

class Event : public ProcessModelNode {

public:
  Event(const std::string &uuid, const std::string &name,
        const EventDefinition type = EventDefinition::NONE);
  Event(const Event &event);
  virtual ~Event(){};

  COPY_SUPPORT_FUNCTIONS(Event);

  void attachEventReference(std::shared_ptr<ModelEvent> event_reference);
  boost::signals2::connection attachToEvent(
      boost::function<void(const std::shared_ptr<ModelEvent> &)> callback);

  void trigger();

  EventDefinition getEventType();
  std::string getEventUuid();
  std::string getEventName();

  std::string to_string(EventDefinition e);

protected:
  std::shared_ptr<ModelEvent> event_reference_;
  EventDefinition event_type_;
};

} /* namespace trace */

#endif /* TRACE_MODEL_EVENT_HPP_ */
