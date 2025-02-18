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

#include "trace/token.h"

#include "trace/model.h"
// #include "trace/timeline.h"

#include <trace/model/BoundaryEvent.hpp>
#include <trace/model/CallActivity.hpp>
#include <trace/model/IntermediateCatchEvent.hpp>
#include <trace/model/SequenceFlow.hpp>
#include <trace/model/Task.hpp>

#include <trace/model/ExclusiveGateway.hpp>
#include <trace/model/InclusiveGateway.hpp>

#include <trace/model/SignalIntermediateCatchEvent.hpp>
#include <trace/model/SignalStartEvent.hpp>

// Boost
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

// C++11
#include <algorithm>
#include <cstdlib>
#include <sstream>

namespace trace {

Token::Token(Model *instance, const std::string &flow_node_uuid,
             bool is_resuming)
    : Object("token",
             "Token_" + std::to_string(boost::uuids::hash_value(
                            boost::uuids::random_generator()())),
             ""),
      is_active_(false), is_interrupted_(false), is_resuming_(is_resuming),
      instance_(instance) {
  start(flow_node_uuid);
}

Token::~Token() { TRACE_LOG_INFO(uuid_, "Deallocating " << uuid_ << "."); }

void Token::start(const std::string &uuid) {

  if (!isActive()) {

    setCurrentNode(uuid);

    {
      // std::lock_guard<std::mutex> lock_node(current_node_mutex_);
      if (current_node_ == nullptr) {
        TRACE_LOG_FATAL(
            uuid_, "Cannot start token, because provided node UUID is invalid. "
                   "This should never happen.");
        std::exit(EXIT_FAILURE);
      }
    }

    {
      std::lock_guard<std::mutex> lock_active(is_active_mutex_);
      is_active_ = true;
    }

    internal_future_ =
        std::async(std::launch::async, &trace::Token::loop, this);

    TRACE_LOG_INFO(uuid_, "Spun up internal thread for " << getUuid() << ".");
  } else {
    TRACE_LOG_ERROR(uuid_,
                    "Cannot start token, because it has already been started.");
  }
}

void Token::loop() {
  while (isActive()) {
    try {
      activity();
    } catch (const std::exception &e) {
      publishTerminationNotification(Types::ExitStatusCode::EXCEPTION);
      deactivate();
      TRACE_LOG_ERROR(uuid_,
                      "Token terminated with an exception: " << e.what());
      return;
    }
  }
  TRACE_LOG_INFO(
      uuid_, getUuid() << " token about to publish termination notification..");
  publishTerminationNotification(Types::ExitStatusCode::OK);
  TRACE_LOG_INFO(uuid_, getUuid() << " terminated normally.");
}

void Token::interrupt() {
  {
    std::unique_lock<std::mutex> lock(is_active_mutex_);
    is_active_ = false;
  }

  std::shared_ptr<Activity> node;
  auto current_node = getCurrentNode();
  {
    node = std::dynamic_pointer_cast<Activity>(current_node);
    if (node == nullptr) {
      current_node->terminate();
    }
  }
  if (node != nullptr) {
    is_interrupted_ = true;
    node->interrupt();
  }
}

void Token::terminate() {
  {
    std::unique_lock<std::mutex> lock(is_active_mutex_);
    is_active_ = false;
  }
  TRACE_LOG_WARN(uuid_, "Token calling teriminate.....");
  auto current_node = getCurrentNode();
  if (current_node_ != nullptr) {
    current_node->terminate();
  } else {
    TRACE_LOG_ERROR(uuid_, "Nothing to terminate!");
  }

  TRACE_LOG_WARN(uuid_, "Token Node was teriminated.....");

  internal_future_.wait();

  TRACE_LOG_WARN(uuid_, "Token teriminated.....");
}

bool Token::isActive() {
  std::lock_guard<std::mutex> lock(is_active_mutex_);
  return is_active_;
}

bool Token::isTerminated() {
  std::lock_guard<std::mutex> lock(is_active_mutex_);
  // TRACE_LOG_INFO(uuid_, "Active? " << std::to_string(is_active_));
  if (this->is_active_ == false) {
    if (this->internal_future_.valid()) {
      auto status =
          this->internal_future_.wait_for(std::chrono::milliseconds(0));
      return (status == std::future_status::ready);
    } else {
      return true;
    }
  }
  return false;
  // auto status = internal_future_.wait_for(std::chrono::milliseconds(0));
  // TRACE_LOG_INFO(uuid_, "Joinable? " << std::to_string(status ==
  // std::future_status::ready));

  // return (!is_active_ && (status == std::future_status::ready));
}

void Token::publishTerminationNotification(
    Types::ExitStatusCode exit_status_code) {
  switch (exit_status_code) {
  case Types::ExitStatusCode::FALSE_ALARM:
    throw std::runtime_error("A false alarm shoud never happen!");
    break;
  case Types::ExitStatusCode::OK:
    TRACE_LOG_INFO(uuid_, "Publishing: " << getUuid() << " owned "
                                         << getCurrentNodeUuid()
                                         << ", which exited successfully.");
    break;
  case Types::ExitStatusCode::EXCEPTION:
    TRACE_LOG_INFO(uuid_, "Publishing: "
                              << getUuid() << " owned " << getCurrentNodeUuid()
                              << ", which exited with an exception.");
    break;
  }
  // token_termination_event_publisher_.publish(event);
  this->instance_->notifyOfTokenTermination(getUuid(), getCurrentNodeUuid(),
                                            exit_status_code);
}

void Token::setCurrentNode(const std::string &uuid) {

  TRACE_LOG_INFO(uuid_, "in set current node");
  auto next_node = this->instance_->getNodeByUuid(uuid);

  TRACE_LOG_INFO(uuid_, "got next node");
  std::lock_guard<std::mutex> lock(current_node_mutex_);
  if (std::dynamic_pointer_cast<Gateway>(next_node) == nullptr) {
    // All non-Gateways are clones
    current_node_ = next_node->instantiate();
  } else {

    TRACE_LOG_INFO(uuid_, "casted to gatway and waiting on mutex");
    // All Gateways are unique
    // std::lock_guard<std::mutex> lock(current_node_mutex_);
    TRACE_LOG_INFO(uuid_, "got mutex");
    current_node_ = next_node;
    TRACE_LOG_INFO(uuid_, "set?");
  }

  TRACE_LOG_INFO(uuid_, "here?");

  if (current_node_) {
    TRACE_LOG_INFO(uuid_,
                   "Token is set to own " << current_node_->uuid() << ".");
    this->instance_->punchIn(uuid_, uuid);
  } else {
    TRACE_LOG_INFO(uuid_, "Token is set to not own a node.");
    this->instance_->punchOut(uuid_);
  }
  // REVIEW: throw exception if current_node_ is nullptr?
}

std::shared_ptr<ProcessModelNode> Token::getCurrentNode() {
  std::lock_guard<std::mutex> lock(current_node_mutex_);
  return current_node_;
}

void Token::followOutputPorts(
    const std::string &node_uuid,
    const std::unordered_map<std::string, std::shared_ptr<SequenceFlow>>
        &outgoing_links,
    std::string &next_uuid) {
  TRACE_LOG_INFO(uuid_,
                 "Following outbound flows from node (" << node_uuid << ").");

  auto xor_gateway = std::dynamic_pointer_cast<ExclusiveGateway>(
      this->instance_->getNodeByUuid(node_uuid));
  bool is_xor_gateway = (xor_gateway != nullptr);

  TRACE_LOG_INFO(uuid_, "(is_xor_gateway) = (" << is_xor_gateway << ")");

  auto or_gateway = std::dynamic_pointer_cast<InclusiveGateway>(
      this->instance_->getNodeByUuid(node_uuid));
  bool is_or_gateway = (or_gateway != nullptr);

  TRACE_LOG_INFO(uuid_, "(is_or_gateway) = (" << is_or_gateway << ")");

  // 1. First, check if we have a XOR or OR gateway with a default outgoing flow
  // NOTE: Default flows are only followed if and only if all other conditional
  // flows are FALSE
  std::string default_flow_uuid;

  if (is_xor_gateway) {
    default_flow_uuid = xor_gateway->getDefaultOutputFlow();
  } else if (is_or_gateway) {
    default_flow_uuid = or_gateway->getDefaultOutputFlow();
  }
  TRACE_LOG_INFO(uuid_, "(default_flow_uuid) = (" << default_flow_uuid << ")");

  std::string default_flow_next_uuid;

  if (!default_flow_uuid.empty()) {
    auto match = std::find_if(
        outgoing_links.begin(), outgoing_links.end(),
        [&default_flow_uuid](
            const std::unordered_map<
                std::string, std::shared_ptr<SequenceFlow>>::value_type &vt) {
          return vt.second->uuid() == default_flow_uuid;
        });
    if (match != outgoing_links.end()) {
      default_flow_next_uuid = (*match).first;
    }
  }
  TRACE_LOG_INFO(uuid_, "(default_flow_next_uuid) = (" << default_flow_next_uuid
                                                       << ")");

  // 2. Follow any true outbound flows
  bool is_branching = false;
  for (const auto &flow_kv : outgoing_links) {
    if (flow_kv.first != default_flow_next_uuid) {
      bool is_flow_true = false;
      try {
        is_flow_true = flow_kv.second->evaluateExpression();
      } catch (const std::runtime_error &e) {
        // NOTE: Assumption, an invalid expression is equivalent to a false
        // flow; otherwise, throw the error!
        TRACE_LOG_WARN(
            uuid_, "Error evaluating expression on outbound flow from node ("
                       << node_uuid << "). Treating as false.");
        TRACE_LOG_WARN(uuid_, "Evaluator reports: " << e.what());
      }

      TRACE_LOG_INFO(uuid_, "Is flow true? " << is_flow_true);

      if (is_flow_true) {
        if (is_branching) {
          this->instance_->emitNewToken(flow_kv.first);
          this->instance_->getNodeByUuid(flow_kv.first)
              ->triggerInputPort(node_uuid);
        } else {
          next_uuid = flow_kv.first;
          is_branching = true;
          this->instance_->getNodeByUuid(flow_kv.first)
              ->triggerInputPort(node_uuid);
          if (is_xor_gateway) {
            // XOR gateways only follow one true outbound link
            return;
          }
        }
      }
    }
  }

  // 3. If no true outbound flows were followed, then follow default if exists
  if (next_uuid.empty() && !default_flow_next_uuid.empty()) {
    next_uuid = default_flow_next_uuid;
    this->instance_->getNodeByUuid(next_uuid)->triggerInputPort(node_uuid);
    return;
  }

  TRACE_LOG_INFO(uuid_, "No output ports from " << node_uuid << ".");
}

std::vector<std::string> Token::getAttachedSignals() {
  std::vector<std::string> signals;
  if (isCurrentNodeValid()) {
    // std::lock_guard<std::mutex> lock(current_node_mutex_);
    auto node = getCurrentNode();

    std::shared_ptr<Activity> activity =
        std::dynamic_pointer_cast<Activity>(node);
    if (activity != nullptr) {
      for (auto kv_pair : activity->listBoundaryEvents()) {
        if (kv_pair.second->getEventType() == EventDefinition::SIGNAL) {
          signals.push_back(kv_pair.second->getEventUuid() + "," +
                            kv_pair.second->getEventName());
        }
      }
      return signals;
    }

    std::shared_ptr<SignalIntermediateCatchEvent> catch_event =
        std::dynamic_pointer_cast<SignalIntermediateCatchEvent>(node);
    if (catch_event != nullptr) {
      TRACE_LOG_INFO(uuid_, "Found signal: " << catch_event->getEventUuid()
                                             << ","
                                             << catch_event->getEventName());
      signals.push_back(catch_event->getEventUuid() + "," +
                        catch_event->getEventName());
      return signals;
    }

    std::shared_ptr<SignalStartEvent> start_event =
        std::dynamic_pointer_cast<SignalStartEvent>(node);
    if (start_event != nullptr) {
      signals.push_back(start_event->getEventUuid() + "," +
                        start_event->getEventName());
      return signals;
    }
  }

  return signals;
}

void Token::activity() {

  if (isCurrentNodeValid()) {

    std::string node_uuid = getCurrentNodeUuid();
    TRACE_LOG_INFO(uuid_, "I promise that I, "
                              << getUuid() << ", am working on " << node_uuid
                              << "!");

    auto ns = this->instance_->getParentInfoByChildUuid(current_node_->uuid());
    auto uuid_or_name = (ns.second.empty()) ? ns.first : ns.second;

    // Timeline::instance().start_activity(uuid_or_name, current_node_->uuid(),
    //                                     current_node_->name());
    auto outcome = current_node_->execute(this->instance_, this->is_resuming_);
    // Timeline::instance().end_activity(current_node_->uuid());
    this->is_resuming_ = false;

    TRACE_LOG_DEBUG(uuid_, "outcome ok? " << std::boolalpha << outcome.ok());
    TRACE_LOG_DEBUG(uuid_, "outcome interrupted? " << std::boolalpha
                                                   << outcome.interrupted());
    TRACE_LOG_DEBUG(uuid_, "outcome cancelled? " << std::boolalpha
                                                 << outcome.cancelled());
    TRACE_LOG_DEBUG(uuid_,
                    "outcome error? " << std::boolalpha << outcome.error());

    // Evaluate the outcome
    std::string next_uuid;
    if (outcome.ok()) {
      TRACE_LOG_INFO(uuid_, outcome.details());
      auto outgoing_links = getCurrentNode()->listOutputPorts();
      TRACE_LOG_INFO(uuid_, node_uuid << " has " << outgoing_links.size()
                                      << " output ports.");
      followOutputPorts(node_uuid, outgoing_links, next_uuid);
    } else if (outcome.interrupted()) {
      TRACE_LOG_INFO(uuid_, outcome.details());
      try {
        const std::string boundary_event_uuid =
            outcome.result("boundary_event_uuid");
        TRACE_LOG_INFO(uuid_, "Interrupted by a boundary event ("
                                  << boundary_event_uuid << ").");
        if (!boundary_event_uuid.empty()) {
          followOutputPorts(boundary_event_uuid,
                            this->instance_->getNodeByUuid(boundary_event_uuid)
                                ->listOutputPorts(),
                            next_uuid);
        }
      } catch (const std::out_of_range &e) {
        TRACE_LOG_INFO(uuid_, "Not interrupted by a boundary event.");
      }
    } else if (outcome.cancelled()) {
      TRACE_LOG_WARN(uuid_, outcome.details());
    } else if (outcome.error()) {
      TRACE_LOG_WARN(uuid_, outcome.details());
      std::shared_ptr<Activity> node =
          std::dynamic_pointer_cast<Activity>(getCurrentNode());

      std::vector<std::string> uuids;
      for (const auto &kv : node->listBoundaryEvents()) {
        if (kv.second->getEventType() == EventDefinition::ERROR) {
          uuids.push_back(kv.first);
        }
        // std::cout << "Key:[" << n.first << "] Value:[" << n.second << "]\n";
      }

      if (uuids.size() == 1) {
        followOutputPorts(
            uuids[0],
            this->instance_->getNodeByUuid(uuids[0])->listOutputPorts(),
            next_uuid);

      } else {
        TRACE_LOG_ERROR(uuid_,
                        "This activity has "
                            << uuids.size()
                            << " error boundary events!!! Only expecting 1.");

        throw std::runtime_error("Error boundary event not caught!");
      }

    } else { // if (something else) {
      TRACE_LOG_ERROR(uuid_, outcome.details());
      throw std::runtime_error(outcome.details());
    }

    // Handle the next element, if any
    if (next_uuid.empty()) {
      deactivate();
    } else {
      TRACE_LOG_INFO(uuid_, "Next node to be handled by ["
                                << getUuid() << "] is [" << next_uuid << "]");
      setCurrentNode(next_uuid);
    }

  } else {
    deactivate();
  }
}

void Token::deactivate() {
  std::lock_guard<std::mutex> lock(is_active_mutex_);
  is_active_ = false;
}

bool Token::isOverwatch() {
  auto node = getCurrentNode();
  if (std::dynamic_pointer_cast<Process>(node) != nullptr) {
    return true;
  }
  if (std::dynamic_pointer_cast<CallActivity>(node) != nullptr) {
    return true;
  }
  return false;
}

bool Token::isCurrentNodeValid() {
  TRACE_LOG_INFO(uuid_, "Checking if current node is valid.");
  std::lock_guard<std::mutex> lock(current_node_mutex_);
  return (current_node_ != nullptr);
}

std::string Token::getCurrentNodeUuid() {
  // std::lock_guard<std::mutex> lock(current_node_mutex_);
  // if (current_node_) {
  //   return (current_node_->uuid());
  // } else {
  //   return "NODE_WITHOUT_VALID_UUID";
  // }
  std::lock_guard<std::mutex> lock(current_node_mutex_);
  return (current_node_ != nullptr) ? current_node_->uuid()
                                    : "NODE_WITHOUT_VALID_UUID";
}

std::string Token::getCurrentNodeLocalName() {
  // std::lock_guard<std::mutex> lock(current_node_mutex_);
  // if (current_node_) {
  //   return (current_node_->name());
  // } else {
  //   return "NODE_WITHOUT_VALID_NAME";
  // }
  std::lock_guard<std::mutex> lock(current_node_mutex_);
  return (current_node_ != nullptr) ? current_node_->name()
                                    : "NODE_WITHOUT_VALID_NAME";
}

} // namespace trace
