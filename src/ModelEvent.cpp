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

#include <trace/ModelEvent.hpp>

namespace trace {

ModelEvent::ModelEvent(const std::string &uuid, const std::string &name,
                       const EventDefinition type)
    : Object("event", uuid, name), event_type_(type) {}

ModelEvent::~ModelEvent() { signal_.disconnect_all_slots(); }

EventDefinition ModelEvent::getEventType() { return event_type_; }

std::string ModelEvent::getEventUuid() {
  return uuid_; // getUuid(); //event_uuid_;
}

std::string ModelEvent::getEventName() {
  return name_; // getLocalName(); // event_name_;
}

void ModelEvent::trigger() {
  auto event = std::shared_ptr<ModelEvent>(this, [](ModelEvent *) {});
  signal_(event);
}

boost::signals2::connection ModelEvent::attachEventResultCallback(
    boost::function<void(const std::shared_ptr<ModelEvent> &)> callback) {
  return signal_.connect(callback);
}

} // namespace trace
