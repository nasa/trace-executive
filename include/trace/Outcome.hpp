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

#ifndef TRACE_OUTCOME_HPP_
#define TRACE_OUTCOME_HPP_

#include <string>
#include <unordered_map>

#include <trace/types/StatusCode.hpp>

namespace trace {

class Outcome {
public:
  std::string details() const;

  bool ok() const;
  bool interrupted() const;
  bool cancelled() const;
  bool error() const;

  std::string result(const std::string &key);
  void add_result(const std::string &key, const std::string &value);
  std::unordered_map<std::string, std::string> results() const;

  Outcome();
  Outcome(StatusCode code, const std::string &details);

private:
  StatusCode code_;
  std::string details_;
  std::unordered_map<std::string, std::string> results_;
};

} // namespace trace

#endif
