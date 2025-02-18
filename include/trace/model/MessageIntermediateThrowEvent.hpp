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

#ifndef MODEL_MESSAGEINTERMEDIATETHROWEVENT_HPP_
#define MODEL_MESSAGEINTERMEDIATETHROWEVENT_HPP_

#include <trace/model/IntermediateThrowEvent.hpp>

namespace trace {

class MessageIntermediateThrowEvent : public IntermediateThrowEvent {

public:
  MessageIntermediateThrowEvent(const std::string &uuid,
                                const std::string &name);
  virtual ~MessageIntermediateThrowEvent();

  COPY_SUPPORT_FUNCTIONS(MessageIntermediateThrowEvent);
};

} /* namespace trace */

#endif /* MODEL_MESSAGEINTERMEDIATETHROWEVENT_HPP_ */
