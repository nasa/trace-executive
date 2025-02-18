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

#ifndef TRACE_MODEL_TASK_HPP_
#define TRACE_MODEL_TASK_HPP_

#include <trace/Types.hpp>
#include <trace/model/Activity.hpp>

namespace trace {

class Task : public Activity {

public:
  Task(const std::string &uuid, const std::string &name,
       const Types::TaskType &type = Types::TaskType::UNDEFINED);
  Task(const Task &task);
  virtual ~Task();

  COPY_SUPPORT_FUNCTIONS(Task);

  Types::TaskType getType();

protected:
  Types::TaskType type_;
};

} /* namespace trace */

#endif /* TRACE_MODEL_TASK_HPP_ */
