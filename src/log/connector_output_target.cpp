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

#include "trace/log/connector_output_target.hpp"

#include "trace/connector/ConnectorManager.hpp"

#include <iostream>

namespace trace {
namespace log {

ConnectorOutputTarget::ConnectorOutputTarget(const std::string &connector_id)
    : connector_id_(connector_id) {}

void ConnectorOutputTarget::write(const Message &message) {

  auto connector = ConnectorManager::instance().connector(this->connector_id_);
  if (connector != nullptr) {

    // FIXME: break connector API to work around this hack

    trace::SeverityLevel severity_level = trace::SeverityLevel::DEBUG;
    if (message.severity_level() == SeverityLevel::info()) {
      severity_level = trace::SeverityLevel::INFO;
    } else if (message.severity_level() == SeverityLevel::warn()) {
      severity_level = trace::SeverityLevel::WARNING;
    } else if (message.severity_level() == SeverityLevel::error()) {
      severity_level = trace::SeverityLevel::ERROR;
    } else if (message.severity_level() == SeverityLevel::fatal()) {
      severity_level = trace::SeverityLevel::FATAL;
    }

    //

    connector->log(severity_level, message.function_name(), message.file_name(),
                   message.line_number(),
                   "(" + message.tag() + ") " + message.content());
  }
}

} // namespace log
} // namespace trace