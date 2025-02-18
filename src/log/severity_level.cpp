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

#include "trace/log/severity_level.hpp"

#include <stdexcept>

namespace trace {
namespace log {

SeverityLevel::SeverityLevel(const SeverityLevel::Type type)
    : internal_type_(type) {
  // no-op
}

SeverityLevel SeverityLevel::debug() { return SeverityLevel(Type::DEBUG); }

SeverityLevel SeverityLevel::info() { return SeverityLevel(Type::INFO); }

SeverityLevel SeverityLevel::warn() { return SeverityLevel(Type::WARN); }

SeverityLevel SeverityLevel::error() { return SeverityLevel(Type::ERROR); }

SeverityLevel SeverityLevel::fatal() { return SeverityLevel(Type::FATAL); }

std::string SeverityLevel::toKeyword() const {
  switch (internal_type_) {
  case Type::DEBUG:
    return "DD";
  case Type::INFO:
    return "II";
  case Type::WARN:
    return "WW";
  case Type::ERROR:
    return "EE";
  case Type::FATAL:
    return "FF";
  }
  throw std::logic_error("Internal type is unhandeled.");
}

std::string SeverityLevel::toAnsiColorCode() const {
  switch (internal_type_) {
  case Type::DEBUG:
    return "\033[36m";
  case Type::INFO:
    return "\033[0m";
  case Type::WARN:
    return "\033[33m";
  case Type::ERROR:
    return "\033[31m";
  case Type::FATAL:
    return "\033[31m";
  }
  throw std::logic_error("Internal type is unhandeled.");
}

bool SeverityLevel::operator==(const SeverityLevel &severity_level) const {
  return (internal_type_ == severity_level.internal_type_);
}

} // namespace log
} // namespace trace