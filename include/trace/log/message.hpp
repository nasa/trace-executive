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

#include "trace/log/location.hpp"
#include "trace/log/severity_level.hpp"
#include "trace/log/timestamp.hpp"

#include <chrono>
#include <string>

namespace trace {
namespace log {

/**
 * @brief
 * A message with all of the metadata and content intended for logging.
 *
 * Contains severity level (DEBUG, INFO, ...), file and function name,
 * line number, a tag, and the content of the message.
 */
class Message {

public:
  /**
   * @brief
   * Constructs a message with a timestamp of its creation.
   *
   * @param severity_level The severity level of this message
   * @param location The function, file, and line number of this message
   * @param tag The owner tag of this message
   * @param content The content (not metadata) of this message
   */
  Message(const SeverityLevel &severity_level, const Timestamp &timestamp,
          const Location &location, const std::string &tag,
          const std::string &content);

  /**
   * @brief
   * Returns a (default) string representation of this message.
   */
  std::string toString() const;

  /**
   * @brief
   * Returns the severity level of this message
   */
  SeverityLevel severity_level() const;

  /**
   * @brief
   * Returns the creation timestamp of this message.
   */
  Timestamp timestamp() const;

  /**
   * @brief
   * Returns the file name in which this message was created.
   */
  std::string file_name() const;

  /**
   * @brief
   * Returns the function name in which this message was created.
   */
  std::string function_name() const;

  /**
   * @brief
   * Returns the line number on which this message was created.
   */
  int line_number() const;

  /**
   * @brief
   * Returns the owner tag of this message.
   */
  std::string tag() const;

  /**
   * @brief
   * Returns the message content.
   */
  std::string content() const;

private:
  SeverityLevel severity_level_;
  Timestamp timestamp_;
  Location location_;
  std::string tag_, content_;
};

} // namespace log
} // namespace trace