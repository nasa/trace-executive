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

#include <string>

namespace trace {
namespace log {

/**
 * @brief
 * Stores the function name, file name, and line number of a
 * a specific location in the source code. Intended for creating
 * a Message for logging.
 *
 * By default, it'll be displayed in the logs as:
 * [in function_name() at file_name:line_number]
 */
class Location {

public:
  /**
   * @brief
   * Constructs an object with a fixed function name, file name,
   * and location.
   *
   * @param function_name Name of the function, usually provided by __FUNCTION__
   * @param file_name  Name of the file, usually provided by __FILE__
   * @param line_number Line number in the file, usually provided by __LINE__
   */
  Location(const std::string &function_name, const std::string &file_name,
           const int line_number);

  /**
   * @brief
   * Returns the function name at this location.
   */
  std::string function_name() const;

  /**
   * @brief
   * Returns the file name at this location.
   */
  std::string file_name() const;

  /**
   * @brief
   * Returns the line number at this location.
   */
  int line_number() const;

private:
  std::string function_name_, file_name_;
  int line_number_;
};

} // namespace log
} // namespace trace