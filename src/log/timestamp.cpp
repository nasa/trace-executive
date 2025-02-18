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

#include "trace/log/timestamp.hpp"

namespace trace {
namespace log {

Timestamp::Timestamp(const std::string &time_format)
    : default_time_format_(time_format),
      time_point_(std::chrono::system_clock::now()) {}

std::chrono::system_clock::time_point Timestamp::time_point() const {
  return time_point_;
}

std::string Timestamp::toString() const {
  return toString(default_time_format_);
}

std::string Timestamp::toString(const std::string &time_format) const {
  std::time_t time_instance = std::chrono::system_clock::to_time_t(time_point_);
  auto time_info = std::localtime(&time_instance);

  std::string buffer = time_format;
  std::size_t bytes_written = 0;

  do {
    buffer.resize(buffer.size() * 2);
    bytes_written = std::strftime(&buffer[0], buffer.size(),
                                  time_format.c_str(), time_info);
  } while (bytes_written == 0);

  return buffer;
}

} // namespace log
} // namespace trace