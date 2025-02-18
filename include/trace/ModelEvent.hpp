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

#ifndef TRACE_MODELEVENT_HPP_
#define TRACE_MODELEVENT_HPP_

#include <trace/Object.hpp>

// std
#include <memory>
#include <string>

// boost
#include <boost/function.hpp>
#include <boost/signals2.hpp>

// local
#include <trace/Types.hpp>
#include <trace/types/EventDefinition.hpp>

namespace trace {

class ModelEvent : public Object {

public:
  ModelEvent(const std::string &uuid, const std::string &name,
             const EventDefinition type);
  virtual ~ModelEvent();

  EventDefinition getEventType();
  std::string getEventUuid();
  std::string getEventName();

  boost::signals2::connection attachEventResultCallback(
      boost::function<void(const std::shared_ptr<ModelEvent> &)> callback);
  void trigger();

private:
  std::string event_uuid_, event_name_;
  EventDefinition event_type_;

  boost::signals2::signal<void(const std::shared_ptr<ModelEvent> &)> signal_;
};

} // namespace trace

#endif
