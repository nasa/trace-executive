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

#ifndef MODEL_BOUNDARYEVENT_HPP_
#define MODEL_BOUNDARYEVENT_HPP_

#include <trace/model/Event.hpp>

namespace trace {

class Activity;

class BoundaryEvent : public Event {

  friend Activity;

public:
  BoundaryEvent(const std::string &uuid, const std::string &name,
                const EventDefinition &type = EventDefinition::NONE);
  BoundaryEvent(const BoundaryEvent &boundary_event);
  virtual ~BoundaryEvent();

  COPY_SUPPORT_FUNCTIONS(BoundaryEvent);

  void attachedToUuid(const std::string &uuid);
  void cancelActivity(bool cancel_activity);

  std::string getAttachedToUuid();
  bool getCancelActivity();

protected:
  std::string attached_to_uuid_;
  bool cancel_activity_ = true;
};

} /* namespace trace */

#endif /* MODEL_BOUNDARYEVENT_HPP_ */
