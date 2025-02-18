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

namespace trace {
namespace log {

/**
 * @brief
 * A log target that outputs messages to the connector's logger.
 */
class ConnectorOutputTarget : public LogTarget {

public:
  ConnectorOutputTarget(const std::string &connector_id);

  /**
   * @brief
   * Passes the message to the connector for logging.
   *
   * @param message Message to be written by connectors.
   */
  void write(const Message &message) override;

private:
  std::string connector_id_;
};

} // namespace log
} // namespace trace