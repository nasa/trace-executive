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

#include <trace/connector/ConnectorManager.hpp>

#include <stdexcept>

namespace trace {

ConnectorManager &ConnectorManager::instance() {
  static ConnectorManager static_instance;
  return static_instance;
}

void ConnectorManager::load(const std::string &library_path, bool is_default) {

  // 1. load connector library dynamically
  // void* handle = dlopen(library_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
  void *handle = dlopen(library_path.c_str(), RTLD_LAZY);
  if (handle == nullptr) {
    TRACE_LOG_ERROR("ConnectorManager",
                    "Unable to load library: " << dlerror());
    return;
  }
  library_handles_[library_path] = handle;

  // 2. inspect the connector
  ConnectorDetails *info =
      reinterpret_cast<ConnectorDetails *>(dlsym(handle, "exports"));

  TRACE_LOG_INFO("ConnectorManager",
                 "Loaded connector: " << info->connector_name << "-"
                                      << info->connector_version);

  ConnectorInterface *iface =
      reinterpret_cast<ConnectorInterface *>(info->initialization_function());

  connectors_[std::string(info->connector_name)] =
      iface; // std::shared_ptr<ConnectorInterface>(iface);

  if (is_default) {
    default_connector_ = iface;
  }

  // 3. initialize the connector
  iface->initialize();
}

void ConnectorManager::unload_all() {
  for (auto kv_pair : connectors_) {
    TRACE_LOG_INFO("ConnectorManager", "connector: " << kv_pair.first);
    kv_pair.second->destroy();
  }
  connectors_.clear();

  for (auto kv_pair : library_handles_) {
    if (dlclose(kv_pair.second) < 0) {
      TRACE_LOG_WARN("ConnectorManager",
                     "Unable to unload library: " << kv_pair.first);
    }
  }
  library_handles_.clear();
}

ConnectorInterface *ConnectorManager::default_connector() {
  return default_connector_;
}

ConnectorInterface *
ConnectorManager::connector(const std::string &connector_name) {
  try {
    return connectors_.at(connector_name);
  } catch (const std::out_of_range & /* e */) {
    return nullptr;
  }
}

} // namespace trace
