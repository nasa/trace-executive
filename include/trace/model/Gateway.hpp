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

#ifndef TRACE_MODEL_GATEWAY_HPP_
#define TRACE_MODEL_GATEWAY_HPP_

#include <trace/model/ProcessModelNode.hpp>

namespace trace {

class Gateway : public ProcessModelNode {

public:
  Gateway(const std::string &uuid, const std::string &name);
  virtual ~Gateway();

  void setDefaultOutputFlow(const std::string &uuid);
  std::string getDefaultOutputFlow();

  COPY_SUPPORT_FUNCTIONS(Gateway);

private:
  std::string default_output_flow_uuid_;
};

} /* namespace trace */

#endif /* TRACE_MODEL_GATEWAY_HPP_ */
