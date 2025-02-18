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

#ifndef TRACE_MODEL_CONNECTORSERVICETASK_HPP_
#define TRACE_MODEL_CONNECTORSERVICETASK_HPP_

#include "trace/model/ServiceTask.hpp"

#include <future>

namespace trace {

class ConnectorServiceTask : public ServiceTask {

public:
  ConnectorServiceTask(const std::string &uuid, const std::string &name,
                       const std::string &connector_id);
  ConnectorServiceTask(const ConnectorServiceTask &connector_service_task);
  virtual ~ConnectorServiceTask();

  COPY_SUPPORT_FUNCTIONS(ConnectorServiceTask);

protected:
  virtual void start();
  virtual void resume();
  virtual void cleanup();
  virtual Outcome activity();
  void wait_for_terminate_signal();

  // std::string connector_id_;
  Outcome connector_activity_outcome_;

  std::promise<Outcome> outcome_promise_;

  std::future<void> wait_for_terminate_signal_future_;
  std::atomic<bool> is_external_terminate_signaled_;
  std::atomic<bool> is_destructing_;
};

} /* namespace trace */

#endif /* TRACE_MODEL_CONNECTORSERVICETASK_HPP_ */
