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

// FIXME: This pragma is not portable outside GCC, use #include guards
#pragma once

// FIXME: use proper include header ordering, see C++ style guide

#include "trace/Outcome.hpp"
#include "trace/log/macros.hpp"
// #include "trace/model.h"
#include "trace/model/flow_element.h"

// std
#include <string>
#include <unordered_map>
#include <vector>

// linux
#include <sys/types.h>

// threading
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <thread>

using PropertyMap = std::unordered_map<std::string, std::string>;

namespace trace // FIXME: we should live in model namespace, not just trace
{

// FIXME: Should be overriding; also, is our intention really to copy the
// current state or a new duplicate? For example, return
// std::make_shared<T>(uuid, sequence flows, etc);
#define COPY_SUPPORT_FUNCTIONS(T)                                              \
  virtual std::shared_ptr<ProcessModelNode> instantiate() override {           \
    return std::make_shared<T>(*this);                                         \
  }

class Model;
class SequenceFlow;

class ProcessModelNode : public model::FlowElement {
public:
  // constructors, destructors
  ProcessModelNode(const std::string &uuid, const std::string &name);
  // FIXME: unnecessary unless we have special copy rules
  ProcessModelNode(const ProcessModelNode &reference);
  virtual ~ProcessModelNode();

  // clone support functions
  // COPY_SUPPORT_FUNCTIONS(ProcessModelNode);
  virtual std::shared_ptr<ProcessModelNode> instantiate() {
    return std::make_shared<ProcessModelNode>(*this);
  }

  // service function
  Outcome execute(Model *instance, bool is_resuming);

  // life cycle support functions
  virtual void terminate();

  bool isActive();

  // FIXME: input/output ports should be added via constructor

  // input port support functions
  void addInputPort(const std::string &uuid);
  void triggerInputPort(const std::string &uuid);
  void resetInputPorts();

  /**
   * @brief Return a list containing UUIDs of all active input ports to this
   * element at runtime.
   *
   * @return std::vector<std::string>
   */
  std::vector<std::string> ListActiveInputPorts();

  // output port support functions
  void addOutputPort(const std::string &uuid,
                     std::shared_ptr<SequenceFlow> &outbound_flow);
  std::unordered_map<std::string, std::shared_ptr<SequenceFlow>>
  listOutputPorts();

  // data store support functions
  void addToDataInputStore(const std::string &name, const std::string &value);
  void assignFromDataStore(const std::string &field, std::string &value);
  void assignFromDataStore(const std::string &field, std::string &value,
                           const std::string &default_value);
  std::unordered_map<std::string, std::string> retrieveDataInputStore();

  std::string getConnectorUuid() const;

  ///
  void addToProperties(const std::string &name, const std::string &value);
  std::string getPropertyByName(const std::string &name) const;

protected:
  virtual void start();
  virtual void resume();
  virtual void cleanup();

  std::unordered_map<std::string, bool> input_ports_;
  std::unordered_map<std::string, std::shared_ptr<SequenceFlow>> output_ports_;

  std::unordered_map<std::string, std::string> data_input_store_;

  std::mutex external_termination_signal_mutex_;
  std::condition_variable external_termination_signaled_;

  std::atomic<bool> external_termination_signaled_for_real_;

  std::mutex is_active_mutex_, input_ports_mutex_;
  bool is_active_ = false;

  // life-cycle support functions
  virtual Outcome activity();

  void deactivate();
  void waitForTermination();

  // implementation
  std::string connector_id_;
  PropertyMap properties_;

  Model *instance_;
};

} // namespace trace
