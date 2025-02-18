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

#include "trace/log/log_target.hpp"
#include "trace/log/standard_output_target.hpp"

#include "trace/log/location.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace trace {
namespace log {

/**
 * @brief
 * An objects that provides the functionality for logging content (e.g., a debug
 * statment) via a set of log targets.
 */
class Logger {

public:
  /**
   * @brief
   * Constructs a logger with some default log target, StandardOutputTarget if
   * not specified.
   */
  Logger(const std::string &uuid = "stdout",
         const std::shared_ptr<LogTarget> default_target =
             std::make_shared<StandardOutputTarget>());

  /**
   * @brief
   * Logs a message to all of the log targets.
   *
   * @param level Severity level of the message
   * @param location Location of the message
   * @param tag Tag (or owner) of the message
   * @param content Content of the message
   */
  void log(const SeverityLevel &level, const Location &location,
           const std::string &tag, const std::string &content);

  void addTarget(const std::string &uuid,
                 const std::shared_ptr<LogTarget> new_target);
  void removeTarget(const std::string &uuid);

private:
  std::mutex target_mutex_;
  std::unordered_map<std::string, std::shared_ptr<LogTarget>> targets_;
};

/**
 * @brief
 * Globally accessible logger used with the logging macros.
 */
extern Logger _global_logger;

} // namespace log
} // namespace trace