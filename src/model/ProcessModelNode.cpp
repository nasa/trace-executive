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

#include "trace/model/ProcessModelNode.hpp"

#include "trace/model/SequenceFlow.hpp"
#include "trace/storage/DataStore.hpp"

#include <chrono>
#include <exception>
#include <iomanip>
#include <iostream>
#include <regex>
#include <stdexcept>

namespace trace {

// constructor && deconstructor

ProcessModelNode::ProcessModelNode(const std::string &uuid,
                                   const std::string &name)
    : FlowElement(uuid, name) {
  external_termination_signaled_for_real_ = false;
  // no-op
}

ProcessModelNode::ProcessModelNode(const ProcessModelNode &reference)
    : FlowElement(reference.uuid(), reference.name()) {
  input_ports_ = reference.input_ports_;
  output_ports_ = reference.output_ports_;
  data_input_store_ = reference.data_input_store_;
  external_termination_signaled_for_real_ = false;
}

ProcessModelNode::~ProcessModelNode() {}

// data store functions

void ProcessModelNode::addToDataInputStore(const std::string &name,
                                           const std::string &value) {
  data_input_store_[name] = value;
}

void ProcessModelNode::assignFromDataStore(const std::string &field,
                                           std::string &value) {
  try {
    value = data_input_store_.at(field);
  } catch (std::out_of_range const &e) {
    throw std::runtime_error("Unable to assign value for [" + field +
                             "] from data store.");
  }
}

void ProcessModelNode::assignFromDataStore(const std::string &field,
                                           std::string &value,
                                           const std::string &default_value) {
  try {
    value = data_input_store_.at(field);
  } catch (std::out_of_range const &e) {
    value = default_value;
  }
}

std::unordered_map<std::string, std::string>
ProcessModelNode::retrieveDataInputStore() {

  std::unordered_map<std::string, std::string> data_input_store;

  for (const auto &kv_pair : data_input_store_) {

    std::string magic_value = kv_pair.second;

    TRACE_LOG_INFO(uuid(), "Processing value (" << magic_value << ") for key ("
                                                << kv_pair.first << ")");
    std::string expanded_value =
        DataStore::instance().FromExpression(magic_value);
    TRACE_LOG_INFO(uuid(), "Using value of (" << expanded_value << ") for key ("
                                              << kv_pair.first << ")");

    data_input_store[kv_pair.first] = expanded_value;
  }

  return data_input_store;
} // namespace trace

// life cycle functions

void ProcessModelNode::terminate() {
  TRACE_LOG_INFO(uuid(), "Base terminate() called.");
  deactivate();
  external_termination_signaled_for_real_ = true;
  external_termination_signaled_.notify_all();

  TRACE_LOG_INFO(uuid(), "Base terminate() finished.");
}

void ProcessModelNode::waitForTermination() {
  std::unique_lock<std::mutex> lock(this->external_termination_signal_mutex_);
  while (!external_termination_signaled_for_real_) {
    external_termination_signaled_.wait(lock);
  }
  lock.unlock();
}

bool ProcessModelNode::isActive() {
  std::lock_guard<std::mutex> lock(is_active_mutex_);
  return is_active_;
}

void ProcessModelNode::start() {
  {
    std::lock_guard<std::mutex> lock(is_active_mutex_);
    is_active_ = true;
  }
  TRACE_LOG_INFO(uuid(), "Base start() called.");
}

void ProcessModelNode::resume() {
  {
    std::lock_guard<std::mutex> lock(is_active_mutex_);
    is_active_ = true;
  }
  TRACE_LOG_INFO(uuid(), "Base resume() called.");
}

void ProcessModelNode::cleanup() {
  // no-op
  TRACE_LOG_INFO(uuid(), "Base cleanup() called.)");
}

Outcome ProcessModelNode::execute(Model *instance, bool is_resuming) {
  // startup
  this->instance_ = instance;

  if (is_resuming) {
    resume();
  } else {
    start();
  }

  // act
  auto outcome = activity();

  // cleanup
  deactivate();
  cleanup();

  this->instance_ = nullptr;
  // report outcome
  return outcome;
}

void ProcessModelNode::deactivate() {
  std::lock_guard<std::mutex> lock(is_active_mutex_);
  is_active_ = false;
}

Outcome ProcessModelNode::activity() {
  TRACE_LOG_INFO(uuid(), "I promise that I, " + name() + " (" + uuid() +
                             "), am working on it!");
  return Outcome(StatusCode::OK, "");
}

// Functions for handling input ports

void ProcessModelNode::addInputPort(const std::string &uuid) {
  input_ports_[uuid] = false;
}

void ProcessModelNode::triggerInputPort(const std::string &uuid) {
  std::lock_guard<std::mutex> lock(input_ports_mutex_);
  if (input_ports_.find(uuid) != input_ports_.end()) {
    TRACE_LOG_INFO(this->uuid(), "Triggering input port (" << uuid << ").");
    input_ports_[uuid] = true;
  } else {
    throw std::runtime_error("Undefined input port for this node.");
  }
}

void ProcessModelNode::resetInputPorts() {
  std::lock_guard<std::mutex> lock(input_ports_mutex_);
  for (auto input_ports_kv_pair : input_ports_) {
    input_ports_[input_ports_kv_pair.first] = false;
  }
}

void ProcessModelNode::addOutputPort(
    const std::string &uuid, std::shared_ptr<SequenceFlow> &outbound_flow) {
  output_ports_[uuid] = outbound_flow;
}

std::unordered_map<std::string, std::shared_ptr<SequenceFlow>>
ProcessModelNode::listOutputPorts() {
  return output_ports_;
}

std::string ProcessModelNode::getConnectorUuid() const {
  return this->connector_id_;
}

void ProcessModelNode::addToProperties(const std::string &name,
                                       const std::string &value) {
  this->properties_[name] = value;
}

std::string ProcessModelNode::getPropertyByName(const std::string &name) const {
  return this->properties_.at(name);
}

std::vector<std::string> ProcessModelNode::ListActiveInputPorts() {
  std::lock_guard<std::mutex> lock(this->input_ports_mutex_);

  std::vector<std::string> active_input_ports;

  for (auto kv_pair : this->input_ports_) {
    if (kv_pair.second == true) {
      active_input_ports.push_back(kv_pair.first);
    }
  }

  return active_input_ports;
}

} /* namespace trace */
