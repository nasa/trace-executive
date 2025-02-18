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

#include <trace/connector/ConnectorInterface.hpp>

namespace trace {

ConnectorInterface::ConnectorInterface() {}

ConnectorInterface::~ConnectorInterface() {}

bool ConnectorInterface::wait_for(
    std::condition_variable &timer_interrupt,
    std::unique_lock<std::mutex> &timer_lock,
    const std::chrono::milliseconds &duration_ms) {
  std::cv_status status = timer_interrupt.wait_for(timer_lock, duration_ms);
  return (status == std::cv_status::timeout);
}

bool ConnectorInterface::wait_until(
    std::condition_variable &timer_interrupt,
    std::unique_lock<std::mutex> &timer_lock,
    const std::chrono::time_point<std::chrono::system_clock> time_utc) {
  std::cv_status status = timer_interrupt.wait_until(timer_lock, time_utc);
  return (status == std::cv_status::timeout);
}

std::string
ConnectorInterface::vector_to_string(const std::vector<std::string> &v) {
  std::string s;
  for (auto &i : v)
    s += i + '\0';
  return s;
}

std::vector<std::string>
ConnectorInterface::string_to_vector(const std::string &s) {
  std::vector<std::string> v;
  size_t i, i0 = 0;
  while ((i = s.find('\0', i0)) != std::string::npos) {
    v.push_back(s.substr(i0, i - i0));
    i0 = i + 1;
  }
  return v;
}

} // namespace trace
