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

#include <trace/Outcome.hpp>

namespace trace {

Outcome::Outcome() : code_(StatusCode::OK) {}

Outcome::Outcome(StatusCode code, const std::string &details)
    : code_(code), details_(details) {}

std::string Outcome::details() const { return details_; }

bool Outcome::ok() const { return code_ == StatusCode::OK; }

bool Outcome::interrupted() const { return code_ == StatusCode::INTERRUPTED; }
bool Outcome::cancelled() const { return code_ == StatusCode::CANCELLED; }

bool Outcome::error() const { return code_ == StatusCode::ERROR; }

std::string Outcome::result(const std::string &key) { return results_.at(key); }

void Outcome::add_result(const std::string &key, const std::string &value) {
  results_[key] = value;
}

std::unordered_map<std::string, std::string> Outcome::results() const {
  return results_;
}

} // namespace trace
