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

#include <fstream>

namespace trace {
namespace log {

/**
 * @brief
 * A log target that outputs messages to a file.
 */
class FileOutputTarget : public LogTarget {

public:
  FileOutputTarget(const std::string &output_file_url);

  virtual ~FileOutputTarget();

  /**
   * @brief
   * Passes the message to a file for logging.
   *
   * @param message Message to be written to a file.
   */
  void write(const Message &message) override;

private:
  std::ofstream file_;
};

} // namespace log
} // namespace trace