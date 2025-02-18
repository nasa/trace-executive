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

#ifndef TRACE_MODEL_FLOW_HPP_
#define TRACE_MODEL_FLOW_HPP_

#include <trace/model/ProcessModelNode.hpp>

namespace trace {

class Flow : public ProcessModelNode {

public:
  Flow(const std::string &uuid, const std::string &name);
  Flow(const Flow &flow);
  virtual ~Flow();

  COPY_SUPPORT_FUNCTIONS(Flow);

  void setSourceAndTargetUuids(const std::string &source_uuid,
                               const std::string &target_uuid);

  std::string getSourceUuid();
  std::string getTargetUuid();

private:
  std::string source_uuid_, target_uuid_;
};

} /* namespace trace */

#endif /* TRACE_MODEL_FLOW_HPP_ */
