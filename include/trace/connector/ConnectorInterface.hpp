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

#ifndef TRACE_CONNECTOR_CONNECTOR_INTERFACE_HPP_
#define TRACE_CONNECTOR_CONNECTOR_INTERFACE_HPP_

#include "trace/Outcome.hpp"
#include "trace/connector/Connector.hpp"
#include "trace/types/SeverityLevel.hpp"

#include <chrono>
#include <functional>
#include <future>
#include <string>
#include <unordered_map>
#include <vector>

using PropertyMap = std::unordered_map<std::string, std::string>;

namespace trace {

/**
 * @brief Interface definition for any Connector that can plug into the
 * executive and be called to execute activites on the system.
 *
 */
class ConnectorInterface {

public:
  ConnectorInterface();
  virtual ~ConnectorInterface();

  virtual void initialize() = 0;

  virtual void destroy() = 0;

  virtual bool check_resource(const std::string &resource_name,
                              const PropertyMap &properties) = 0;

  virtual bool reserve_resource(const std::string &task_uuid,
                                const PropertyMap &properties) = 0;

  virtual std::shared_future<Outcome>
  command_resource(const std::string &task_uuid,
                   const PropertyMap &properties) = 0;

  virtual void abort_command(const std::string &task_uuid,
                             const PropertyMap &properties) = 0;

  virtual void release_resource(const std::string &task_uuid,
                                const PropertyMap &properties) = 0;

  virtual void log(const SeverityLevel level, const std::string &function_name,
                   const std::string &file_name, int line_number,
                   const std::string &msg) = 0;

  virtual std::chrono::milliseconds
  to_connector_time(const std::chrono::milliseconds &duration_us) = 0;

  virtual bool wait_for(std::condition_variable &timer_interrupt,
                        std::unique_lock<std::mutex> &timer_lock,
                        const std::chrono::milliseconds &duration_ms);

  virtual bool
  wait_until(std::condition_variable &timer_interrupt,
             std::unique_lock<std::mutex> &timer_lock,
             const std::chrono::time_point<std::chrono::system_clock> time_utc);

  static std::string vector_to_string(const std::vector<std::string> &v);

  static std::vector<std::string> string_to_vector(const std::string &s);
};
} // namespace trace

#endif
