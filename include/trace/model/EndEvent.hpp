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

#ifndef MODEL_ENDEVENT_HPP_
#define MODEL_ENDEVENT_HPP_

#include <trace/model/Event.hpp>

namespace trace {

class EndEvent : public Event {
public:
  EndEvent(const std::string &uuid, const std::string &name,
           const EventDefinition &type = EventDefinition::NONE);
  virtual ~EndEvent();

  COPY_SUPPORT_FUNCTIONS(EndEvent);

protected:
  virtual Outcome activity();
};

} /* namespace trace */

#endif /* MODEL_ENDEVENT_HPP_ */
