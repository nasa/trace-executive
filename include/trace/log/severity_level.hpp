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
 * An object that represents one of five possible severity levels (debug, info,
 * warn, error, or fatal). Severity levels can be converted to short hand
 * (keyword) or ANSI color codes.
 */
class SeverityLevel {

public:
  /**
   * @brief
   * Returns a severity level of type DEBUG.
   */
  static SeverityLevel debug();

  /**
   * @brief
   * Returns a severity level of type DEBUG.
   */
  static SeverityLevel info();

  /**
   * @brief
   * Returns a severity level of type DEBUG.
   */
  static SeverityLevel warn();

  /**
   * @brief
   * Returns a severity level of type DEBUG.
   */
  static SeverityLevel error();

  /**
   * @brief
   * Returns a severity level of type DEBUG.
   */
  static SeverityLevel fatal();

  /**
   * @brief
   * Return a string that represents this severity level by some hard-coded
   * keyword. For example, DEBUG's keyword is "DD".
   */
  std::string toKeyword() const;

  /**
   * @brief
   * Return a string that represents this severity level by some hard-coded
   * ANSI color code. For example, ERROR and FATAL are "\033[31m" (red).
   */
  std::string toAnsiColorCode() const;

  /**
   * @brief
   * Returns if the severity levels have the same internal type, i.e. if they
   * are the same.
   *
   * @param severity_level Another severity level for comparison
   */
  bool operator==(const SeverityLevel &severity_level) const;

private:
  /**
   * @brief
   * An enumeration of all allowable severity levels.
   */
  enum class Type : std::size_t { DEBUG, INFO, WARN, ERROR, FATAL };

  /**
   * @brief
   * Constructs a severity level of a specific type.
   *
   * @param type The internal type of this severity level
   */
  SeverityLevel(const Type type);

  Type internal_type_;
};

} // namespace log
} // namespace trace