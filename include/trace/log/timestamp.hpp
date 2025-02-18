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

#pragma once

#include <chrono>
#include <ctime>
#include <string>

namespace trace {
namespace log {

/**
 * @brief
 * A timestamp from the system clock with some default string for formatting.
 */
class Timestamp {

public:
  /**
   * @brief
   * Constructs a time stamp using the system clock on allocation.
   */
  Timestamp(const std::string &time_format = "%F %a %T");

  /**
   * @brief
   * Returns a time_point for this time stamp.
   */
  std::chrono::system_clock::time_point time_point() const;

  /**
   * @brief
   * Returns a string representation of this time stamp using the
   * default time format provided at object construction time.
   */
  std::string toString() const;

  /**
   * @brief
   * Returns a string representation of this time stamp based on the
   * provided time format.
   *
   * @param time_format A string accepted by std::strftime
   */
  std::string toString(const std::string &time_format) const;

private:
  std::string default_time_format_;
  std::chrono::system_clock::time_point time_point_;
};

} // namespace log
} // namespace trace