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

#include "trace/log/message.hpp"

#include <iostream>

namespace trace {
namespace log {

/**
 * @brief
 * Abstract class for all log targets, which at minimum must provide an
 * implementation of the write() function.
 */
class LogTarget {

public:
  /**
   * @brief
   * An abstract function defining the interface for
   * log targets; any log target must override this
   * function for writing a message to its target.
   *
   * @param message Message to be output to the log
   */
  virtual void write(const Message &message) = 0;
};

} // namespace log
} // namespace trace