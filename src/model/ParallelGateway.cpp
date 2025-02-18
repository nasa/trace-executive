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

#include <trace/model/ParallelGateway.hpp>

namespace trace {

ParallelGateway::ParallelGateway(const std::string &uuid,
                                 const std::string &name)
    : Gateway(uuid, name) {}

ParallelGateway::~ParallelGateway() {}

Outcome ParallelGateway::activity() {
  TRACE_LOG_INFO(uuid(), "I promise that I, " << name() << " (" << uuid()
                                              << "), am working on it!");

  std::lock_guard<std::mutex> lock(input_ports_mutex_);
  for (auto key_value_pair : input_ports_) {
    if (!key_value_pair.second) {
      TRACE_LOG_INFO(uuid(), "Port [" << key_value_pair.first << "] is off.");
      // throw std::runtime_error("Start guard of node [" + uuid_ + "] is still
      // on!");
      return Outcome(StatusCode::CANCELLED,
                     "Start guard of node [" + uuid() + "] is still on!");
    }
    TRACE_LOG_INFO(uuid(), "Port [" << key_value_pair.first << "] is on.");
  }

  for (auto key_value_pair : input_ports_) {
    input_ports_[key_value_pair.first] = false;
  }

  return Outcome(StatusCode::OK, "");
}

} /* namespace trace */
