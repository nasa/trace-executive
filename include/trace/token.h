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

// TRACE
#include "trace/ModelEvent.hpp"
#include "trace/Object.hpp"
#include "trace/Types.hpp"
#include "trace/model.h"
#include "trace/model/ProcessModelNode.hpp"

// Boost
#include <boost/signals2/connection.hpp>

// C++11
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <future>
#include <memory>
#include <mutex>
#include <thread>

namespace trace {

class Process;

class Token : public Object {

public:
  explicit Token(Model *instance, const std::string &flow_node_uuid,
                 bool is_resuming);
  ~Token();

  void terminate();
  void interrupt();

  bool isActive();
  bool isTerminated();

  std::string getCurrentNodeUuid();
  std::string getCurrentNodeLocalName();

  std::vector<std::string> getAttachedSignals();

  bool isOverwatch();

  std::shared_ptr<ProcessModelNode> getCurrentNode();

protected:
  void start(const std::string &uuid);

  void deactivate();

  void activity();
  void loop();

  bool isCurrentNodeValid();
  void setCurrentNode(const std::string &uuid);

  void publishTerminationNotification(Types::ExitStatusCode exit_status_code);

  void followOutputPorts(
      const std::string &node_uuid,
      const std::unordered_map<std::string, std::shared_ptr<SequenceFlow>>
          &outgoing_links,
      std::string &next_uuid);

private:
  std::mutex current_node_mutex_;
  std::shared_ptr<ProcessModelNode> current_node_;

  std::future<void> internal_future_;

  std::condition_variable external_termination_signaled_;
  std::mutex is_active_mutex_;

  bool is_active_;
  bool is_interrupted_;
  bool is_resuming_;

  Model *instance_;
};

} // namespace trace
