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

#ifndef PROCESS_ENGINE_INCLUDE_MODEL_ESCALATIONBOUNDARYEVENT_HPP_
#define PROCESS_ENGINE_INCLUDE_MODEL_ESCALATIONBOUNDARYEVENT_HPP_

#include <trace/model/BoundaryEvent.hpp>

namespace trace {

class EscalationBoundaryEvent : public BoundaryEvent {
public:
  EscalationBoundaryEvent(const std::string &uuid, const std::string &name);
  virtual ~EscalationBoundaryEvent();

  COPY_SUPPORT_FUNCTIONS(EscalationBoundaryEvent);
};

} /* namespace trace */

#endif /* PROCESS_ENGINE_INCLUDE_MODEL_ESCALATIONBOUNDARYEVENT_HPP_ */
