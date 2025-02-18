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

#include "trace/log/logger.hpp"

#include "trace/log/timestamp.hpp"

#include <iostream>
#include <typeinfo>

namespace trace {
namespace log {

Logger _global_logger; // = Logger();

Logger::Logger(const std::string &uuid,
               const std::shared_ptr<LogTarget> default_target) {
  targets_[uuid] = default_target;
}

void Logger::log(const SeverityLevel &level, const Location &location,
                 const std::string &tag, const std::string &content) {

  std::unique_lock<std::mutex> lock(this->target_mutex_);

  Message msg(level, Timestamp(), location, tag, content);
  for (auto target : targets_) {
    if (target.first == "stdout" &&
        targets_.find("connector") != targets_.end()) {
      continue; // FIXME: add actual options
    }
    target.second->write(msg);
  }
}

void Logger::addTarget(const std::string &uuid,
                       const std::shared_ptr<LogTarget> new_target) {
  std::unique_lock<std::mutex> lock(this->target_mutex_);
  this->targets_[uuid] = new_target;
}

void Logger::removeTarget(const std::string &uuid) {
  std::unique_lock<std::mutex> lock(this->target_mutex_);
  this->targets_.erase(uuid);
}

} // namespace log
} // namespace trace