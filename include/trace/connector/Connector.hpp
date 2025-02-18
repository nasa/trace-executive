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

#ifndef TRACE_CONNECTOR_CONNECTOR_HPP_
#define TRACE_CONNECTOR_CONNECTOR_HPP_

namespace trace {

class ConnectorInterface;

#define TRACE_CONNECTOR_API_VERSION 3

typedef ConnectorInterface *(*GetConnectorFunction)();

struct ConnectorDetails {
  int api_version;
  const char *file_name;
  const char *class_name;
  const char *connector_name;
  const char *connector_version;
  GetConnectorFunction initialization_function;
};

#define TRACE_CONNECTOR(class_type, connector_name, connector_version)         \
  extern "C" {                                                                 \
  trace::ConnectorInterface *getConnector() {                                  \
    static class_type instance;                                                \
    return &instance;                                                          \
  }                                                                            \
  trace::ConnectorDetails exports = {                                          \
      TRACE_CONNECTOR_API_VERSION, __FILE__,    #class_type, connector_name,   \
      connector_version,           getConnector};                              \
  }

} // namespace trace

#endif
