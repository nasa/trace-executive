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

#include "trace/log/message.hpp"

#include <sstream>

namespace trace {
namespace log {

Message::Message(const SeverityLevel &severity_level,
                 const Timestamp &timestamp, const Location &location,
                 const std::string &tag, const std::string &content)
    : severity_level_(severity_level), timestamp_(timestamp),
      location_(location), tag_(tag), content_(content) {
  // no-op
}

std::string Message::toString() const {
  std::stringstream log_line;

  log_line << "(" << severity_level_.toKeyword() << ") ";
  log_line << "[" << timestamp_.toString() << "] ";
  log_line << "[in " << function_name() << "() at " << file_name() << ":"
           << line_number() << "]: ";
  log_line << "(" << tag_ << ") " << content_;

  return log_line.str();
}

SeverityLevel Message::severity_level() const { return severity_level_; }

Timestamp Message::timestamp() const { return timestamp_; }

std::string Message::file_name() const { return location_.file_name(); }

std::string Message::function_name() const { return location_.function_name(); }

int Message::line_number() const { return location_.line_number(); }

std::string Message::tag() const { return tag_; }

std::string Message::content() const { return content_; }

} // namespace log
} // namespace trace