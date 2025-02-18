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

#ifndef TRACE_MODEL_PROCESS_HPP_
#define TRACE_MODEL_PROCESS_HPP_

#include "trace/model.h"
#include <trace/model/Activity.hpp>

#include <queue>
#include <string>
#include <unordered_map>

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <deque>
#include <memory>

namespace trace {

class Model;

class Process : public Activity {

public:
  Process(const std::string &uuid, const std::string &name);
  Process(const Process &process);
  virtual ~Process();

  COPY_SUPPORT_FUNCTIONS(Process);

  /**
   * @brief Add a child to this process with its UUID as the lookup key.
   *
   * @param uuid The universally unique identifier for the child from BPMN
   * @param node A shared pointer to child
   */
  void addNodeByUuid(const std::string &uuid,
                     std::shared_ptr<ProcessModelNode> node);

  /**
   * @brief Get a child of this process via lookup by UUID.
   *
   * @param uuid The universally unique identifer for the child from BPMN
   * @return std::shared_ptr<ProcessModelNode> A pointer to the child if it
   * blongs to this process. Otherwise, this returns a null pointer.
   */
  std::shared_ptr<ProcessModelNode> getNodeByUuid(const std::string &uuid);

  bool validate() const;

  std::vector<std::string> getStartEventUuids();
  std::vector<std::string> getEndEventUuids();

  virtual void interrupt();
  virtual void terminate() override;
  virtual void terminate(bool is_cancelled,
                         const std::string &terminator_uuid); // override;

  void token_termination_event_callback(const std::string &token_uuid,
                                        const std::string &node_uuid,
                                        const Types::ExitStatusCode &status);

  // FIXME: why is this implemented in the header?
  void printDebugString(const std::string &prefix) {
    TRACE_LOG_INFO(uuid(), prefix << "process : " << uuid());
    for (auto kv_pair : process_definition_) {
      // FIXME: trying to identify classes via these casts is bad practice
      auto process_ptr = std::dynamic_pointer_cast<Process>(kv_pair.second);
      if (process_ptr == nullptr) {
        TRACE_LOG_INFO(uuid(), prefix << "\tnode : " << kv_pair.second->uuid());
      } else {
        process_ptr->printDebugString(prefix + "\t");
      }
    }
  }

  /**
   * @brief Compile a map of children (by their UUIDs) to their active inputs
   * (also by UUID).
   *
   * @return std::unordered_map<std::string, std::vector<std::string>> List of
   * active input UUIDs indexed by child UUID.
   */
  std::unordered_map<std::string, std::vector<std::string>>
  PeekActiveInputsForEachChild();

  /**
   * @brief Activates the listed input ports for the child of this process.
   *
   * @param child_uuid UUID of thei child of this process. If this child does
   * not exist, then this function is a no-op.
   * @param input_port_uuids The list of ports to active on this child.
   */
  void TriggerInputsOnChild(const std::string &child_uuid,
                            const std::vector<std::string> &input_port_uuids);

  /**
   * @brief Fetch the UUIDs of all elements that belong to this process and are
   * parallel gateways.
   *
   * @return std::vector<std::string> List of UUIDs for all parallel gateways in
   * this process.
   */
  std::vector<std::string> FetchParallelGatewayUuids() const;

protected:
  virtual void start();
  virtual void resume();

  Outcome activity();
  virtual void cleanup();

private:
  std::vector<std::string> end_event_uuids_;

  std::unordered_map<std::string, std::shared_ptr<ProcessModelNode>>
      process_definition_;

  std::mutex termination_notification_mutex_;
  std::condition_variable termination_notification_received_;

  class TokenTerminationNotification {

  public:
    TokenTerminationNotification(const std::string &token_uuid,
                                 const std::string &node_uuid,
                                 const Types::ExitStatusCode exit_status)
        : token_uuid_(token_uuid), node_uuid_(node_uuid),
          exit_status_(exit_status) {}

    std::string token_uuid() { return this->token_uuid_; }
    std::string node_uuid() { return this->node_uuid_; }
    Types::ExitStatusCode exit_status() { return this->exit_status_; }

  private:
    std::string token_uuid_, node_uuid_;
    Types::ExitStatusCode exit_status_;
  };

  std::queue<TokenTerminationNotification>
      token_termination_notification_queue_;

  bool is_first_activity_ = true;

  std::atomic<bool> is_cancelled_;

  boost::signals2::connection connection_;
};

} /* namespace trace */

#endif /* TRACE_MODEL_PROCESS_HPP_ */
