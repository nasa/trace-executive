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

#ifndef TRACE_CONNECTOR_CONNECTOR_MANAGER_HPP_
#define TRACE_CONNECTOR_CONNECTOR_MANAGER_HPP_

#include <trace/Object.hpp>

#include <dlfcn.h>

#include <trace/connector/Connector.hpp>
#include <trace/connector/ConnectorInterface.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace trace {

class ConnectorManager : public Object {

public:
  static ConnectorManager &instance();
  virtual ~ConnectorManager(){};

  void load(const std::string &library_path, bool is_default = false);
  void unload_all();

  ConnectorInterface *default_connector();
  ConnectorInterface *connector(const std::string &connector_name);

private:
  ConnectorManager() : Object("connector", "ConnectorManager", ""){};

  std::unordered_map<std::string, void *> library_handles_;
  std::unordered_map<std::string, ConnectorInterface *> connectors_;
  ConnectorInterface *default_connector_ = nullptr;
};

} // namespace trace

#endif
