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

#include <trace/model/Process.hpp>

#include "trace/connector/ConnectorManager.hpp"
#include <trace/model/EndEvent.hpp>
#include <trace/model/ParallelGateway.hpp>
#include <trace/model/StartEvent.hpp>

#include <iostream>

namespace trace {

Process::Process(const std::string &uuid, const std::string &name)
    : Activity(uuid, name) {
  is_cancelled_.store(false);
}

Process::Process(const Process &process) : Activity(process) {
  process_definition_ = process.process_definition_;
  is_cancelled_.store(false);
}

Process::~Process() {}

void Process::interrupt() {
  TRACE_LOG_INFO(uuid(), "This process is being interrupted.");
  is_interrupted_ = true;
  std::vector<std::shared_ptr<ProcessModelNode>> children =
      this->instance_->activeNodesByProcessUuid(uuid());

  TRACE_LOG_INFO(uuid(), "Time to interrupt or terminate (" << children.size()
                                                            << ") children.");

  for (auto child : children) {
    if (std::dynamic_pointer_cast<Activity>(child) != nullptr) {
      TRACE_LOG_INFO(uuid(), "Interrupting child [" << child->uuid() << "]");
      std::static_pointer_cast<Activity>(child)->interrupt();
    } else {
      TRACE_LOG_INFO(uuid(), "Terminating child [" << child->uuid() << "]");
      child->terminate();
    }
  }
}

bool Process::validate() const {

  for (const auto &child : this->process_definition_) {
    try {
      std::string connector_id = child.second->getConnectorUuid(),
                  resource_name = child.second->retrieveDataInputStore().at(
                      "resource_name");

      bool is_valid =
          ConnectorManager::instance()
              .connector(connector_id)
              ->check_resource(resource_name,
                               child.second->retrieveDataInputStore());

      if (is_valid == false) {
        // exit early
        TRACE_LOG_ERROR(
            uuid(), "Child (" << child.first
                              << ") is an invalid implementation in connector ("
                              << connector_id << ")!");
        return false;
      }
      TRACE_LOG_INFO(uuid(), "Child ("
                                 << child.first
                                 << ") is a valid implementation in connector ("
                                 << connector_id << ").");
    } catch (...) {
      TRACE_LOG_INFO(uuid(),
                     "Child (" << child.first << ") is not an implementation.");
      continue;
    }
  }
  return true;
}

void Process::terminate() { this->terminate(true, ""); }

void Process::terminate(bool is_cancelled, const std::string &terminator_uuid) {
  // TRACE_LOG_INFO(uuid(), "This process is being terminated.");
  // std::vector<std::shared_ptr<ProcessModelNode>> children =
  // this->instance_->activeNodesByProcessUuid(uuid_); for (auto child :
  // children) {
  //   std::string child_uuid = child->uuid();
  //   // std::shared_ptr<Token> child_token =
  //   this->instance_->getActiveTokenByNodeUuid(child_uuid);
  //   // if (child_token != nullptr) {
  //     TRACE_LOG_INFO(uuid(), "Terminating child [" << child_uuid << "]");
  //   //   child_token->terminate();
  //   // }
  //   child->terminate();
  // }
  // is_cancelled_.store(is_cancelled);
  // TRACE_LOG_INFO(uuid(), "Terminated all children.");

  TRACE_LOG_INFO(uuid(), "This process is being terminated.");
  std::vector<std::shared_ptr<Token>> child_tokens =
      this->instance_->activeTokensByProcessUuid(uuid());
  for (auto child_token : child_tokens) {
    std::string child_uuid = child_token->getCurrentNodeUuid();
    if (child_uuid == terminator_uuid) {
      continue;
    }
    TRACE_LOG_INFO(uuid(), "Terminating child [" << child_uuid << "]");
    child_token->terminate();
    TRACE_LOG_FATAL(uuid(), "Child terminated successfully.");
    std::cout << std::flush;
  }
  std::cout << std::flush;
  TRACE_LOG_FATAL(uuid(), "Ready to write to is_cancelled.");
  is_cancelled_.store(is_cancelled);
  TRACE_LOG_INFO(uuid(), "Terminated all children.");

  // Activity::terminate();
  TRACE_LOG_INFO(uuid(), "Self terminated.");
}

void Process::token_termination_event_callback(
    const std::string &token_uuid, const std::string &node_uuid,
    const Types::ExitStatusCode &status) {
  auto prototype = getNodeByUuid(node_uuid);
  if (prototype == nullptr) {
    // no-op, not for this (sub)process!
  } else {

    TRACE_LOG_INFO(uuid(), "Received a TokenTerminationNotification ("
                               << token_uuid << ") from " << node_uuid
                               << " with exit status ["
                               << std::to_string(status) << "].");
    TokenTerminationNotification notification(token_uuid, node_uuid, status);

    switch (status) {
    case Types::ExitStatusCode::FALSE_ALARM:
      throw std::runtime_error("A false alarm should never happen!");
      break;
    case Types::ExitStatusCode::OK:
      if (std::dynamic_pointer_cast<EndEvent>(prototype) == nullptr) {
        TRACE_LOG_INFO(uuid(), "Not an end event, notifying self.");
        {
          std::unique_lock<std::mutex> lock(termination_notification_mutex_);
          this->token_termination_notification_queue_.push(notification);
        }
        termination_notification_received_.notify_all();
      } else {
        TRACE_LOG_INFO(uuid(), "End event, notifying self.");
        for (auto end_event_uuid : end_event_uuids_) {
          if (node_uuid.compare(end_event_uuid) == 0) {
            if ((std::static_pointer_cast<EndEvent>(
                     getNodeByUuid(end_event_uuid))
                     ->getEventType() == EventDefinition::TERMINATION)) {
              TRACE_LOG_INFO(uuid(), "Caught a terminating end event ["
                                         << end_event_uuid << "].");
              terminate(false, end_event_uuid);
            }

            {
              std::unique_lock<std::mutex> lock(
                  termination_notification_mutex_);
              this->token_termination_notification_queue_.push(notification);
            }
            termination_notification_received_.notify_all();
            break;
          }
        }
      }
      break;
    case Types::ExitStatusCode::EXCEPTION:
      // HACK: parallel gateways shouldn't really throw exceptions if they are
      // gated
      if (std::dynamic_pointer_cast<ParallelGateway>(
              getNodeByUuid(node_uuid)) == nullptr) {
        {
          std::unique_lock<std::mutex> lock(termination_notification_mutex_);
          this->token_termination_notification_queue_.push(notification);
        }
        termination_notification_received_.notify_all();
      } else {
        TRACE_LOG_WARN(
            uuid(), "Caught exception from a parallel gateway. Don't panic.");
      }
      break;
    }
  }
}

void Process::start() {
  connection_ = this->instance_->attachTokenTerminationEventCallback(
      boost::bind(&Process::token_termination_event_callback, this,
                  boost::placeholders::_1, boost::placeholders::_2,
                  boost::placeholders::_3));
  Activity::start();
  TRACE_LOG_DEBUG(uuid(), "Kicking off all start events...");
  for (auto start_event_uuid : getStartEventUuids()) {
    TRACE_LOG_DEBUG(uuid(), "start event: " << start_event_uuid);
    this->instance_->emitNewToken(start_event_uuid);
  }
  TRACE_LOG_DEBUG(uuid(), "...done.");
}

void Process::resume() {
  connection_ = this->instance_->attachTokenTerminationEventCallback(
      boost::bind(&Process::token_termination_event_callback, this,
                  boost::placeholders::_1, boost::placeholders::_2,
                  boost::placeholders::_3));
  Activity::resume();
}

Outcome Process::activity() {
  TRACE_LOG_INFO(uuid(), "I promise that I, " + name() + " (" + uuid() +
                             "), am working on it!");

  end_event_uuids_ = getEndEventUuids();

  TRACE_LOG_INFO(uuid(),
                 "Waiting for the arrival of a termination notification.");

  // waitForTokenTerminationNotification();

  std::string boundary_event_uuid;

  while (isActive()) {
    std::unique_lock<std::mutex> lock(termination_notification_mutex_);
    // auto status =
    termination_notification_received_.wait_for(lock,
                                                std::chrono::milliseconds(100));

    boundary_event_uuid = interrupting_boundary_event_uuid_;

    while (token_termination_notification_queue_.size() > 0) {

      TokenTerminationNotification notification =
          this->token_termination_notification_queue_.front();
      this->token_termination_notification_queue_.pop();

      bool is_in_wait = true;
      while (is_in_wait) {
        auto active_tokens = this->instance_->activeTokensByProcessUuid(uuid());
        is_in_wait = false;
        for (auto token : active_tokens) {
          std::string node_uuid = token->getCurrentNodeUuid();
          std::string token_uuid = token->getUuid();
          if (node_uuid == notification.node_uuid() &&
              token_uuid == notification.token_uuid()) {
            TRACE_LOG_DEBUG(uuid(),
                            "Child token ("
                                << token_uuid << "," << node_uuid
                                << ") in the notification is still active, so "
                                   "waiting...");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            is_in_wait = true;
            break;
          }
        }
      }

      TRACE_LOG_INFO(uuid(), "Handling a token termination.");
      if (notification.exit_status() == Types::ExitStatusCode::OK) {
        TRACE_LOG_INFO(uuid(), "Counting children...");
        int active_token_count =
            this->instance_->activeTokenCountByProcess(uuid());
        TRACE_LOG_INFO(uuid(),
                       "...found " << active_token_count << " children.");

        if (active_token_count == 0) {
          // I'm the last one left, let's give up
          TRACE_LOG_INFO(uuid(), "Last child left, so shutting down.");
          lock.unlock();

          if (is_interrupted_) {
            TRACE_LOG_INFO(uuid(), "Process was interrupted.");
            auto outcome = Outcome(StatusCode::INTERRUPTED,
                                   "Interrupted before process completed.");
            if (!boundary_event_uuid.empty()) {
              outcome.add_result("boundary_event_uuid", boundary_event_uuid);
            }
            return outcome;
          } else if (is_cancelled_) {
            TRACE_LOG_INFO(uuid(), "Process was cancelled.");
            return Outcome(
                StatusCode::CANCELLED,
                "Cancelled before all overwatched token were terminated!");
          } else {
            TRACE_LOG_INFO(uuid(), "Natural end of my life cycle.");
            TRACE_LOG_INFO(uuid(), "Process terminated successfully");
            return Outcome(StatusCode::OK, "Process terminated successfully");
          }

        } else {
          TRACE_LOG_INFO(
              uuid(),
              "Not terminating until all of my children have terminated. "
                  << active_token_count << " left.");
        }
      } else if (notification.exit_status() ==
                 Types::ExitStatusCode::EXCEPTION) {
        TRACE_LOG_ERROR(
            uuid(),
            "Node termination with an exception occured in this process.");
        lock.unlock();

        TRACE_LOG_WARN(uuid(),
                       "Terminating this process due to reported exception.");
        terminate(false, notification.node_uuid());
        cleanup();
        throw std::runtime_error(
            "Terminating this process due to reported exception.");

        // return Outcome(StatusCode::ERROR, "Node termination with an
        // exception occured in this process.");
      } else {
        TRACE_LOG_ERROR(uuid(), "Something is terribly wrong!");
      }
    }
    lock.unlock();
  }

  if (is_interrupted_) {
    auto outcome = Outcome(StatusCode::INTERRUPTED,
                           "Interrupted before process completed.");
    if (!boundary_event_uuid.empty()) {
      outcome.add_result("boundary_event_uuid", boundary_event_uuid);
    }
    return outcome;
  } else {
    return Outcome(StatusCode::CANCELLED,
                   "Cancelled before all overwatched token were terminated!");
  }
} // namespace trace

void Process::cleanup() {
  connection_.disconnect();
  Activity::cleanup();
}

std::shared_ptr<ProcessModelNode>
Process::getNodeByUuid(const std::string &uuid) {
  try {
    return process_definition_.at(uuid);
  } catch (const std::out_of_range & /* e */) {
    return nullptr;
  }
}

void Process::addNodeByUuid(const std::string &uuid,
                            std::shared_ptr<ProcessModelNode> node) {
  this->process_definition_[uuid] = node;
}

// FIXME: Really shouldn't be using dynamic_pointer_cast to check for type...
std::vector<std::string> Process::getStartEventUuids() {
  std::vector<std::string> start_event_uuids;
  for (auto key_value_pair : process_definition_) {
    if (std::dynamic_pointer_cast<StartEvent>(key_value_pair.second) !=
        nullptr) {
      // if (key_value_pair.first.find("StartEvent_") == 0) {
      TRACE_LOG_INFO(uuid(), uuid() << " identified one of its start events, "
                                    << key_value_pair.first);
      start_event_uuids.push_back(key_value_pair.first);
    }
  }
  return start_event_uuids;
}

// FIXME: Really shouldn't be using dynamic_pointer_cast to check for type...
std::vector<std::string> Process::getEndEventUuids() {
  std::vector<std::string> end_event_uuids;
  for (auto key_value_pair : process_definition_) {
    if (std ::dynamic_pointer_cast<EndEvent>(key_value_pair.second) !=
        nullptr) {
      // if (key_value_pair.first.find("EndEvent_") == 0) { // FIXME: why JP,
      // why?
      end_event_uuids.push_back(key_value_pair.first);
    }
  }
  return end_event_uuids;
}

std::vector<std::string> Process::FetchParallelGatewayUuids() const {
  std::vector<std::string> parallel_gateway_uuids;
  for (const auto &kv_pair : this->process_definition_) {
    if (std::dynamic_pointer_cast<ParallelGateway>(kv_pair.second)) {
      parallel_gateway_uuids.push_back(kv_pair.first);
    }
  }
  return parallel_gateway_uuids;
}

std::unordered_map<std::string, std::vector<std::string>>
Process::PeekActiveInputsForEachChild() {
  std::unordered_map<std::string, std::vector<std::string>>
      active_inputs_by_child;
  for (const auto &kv_pair : process_definition_) {
    if (std::dynamic_pointer_cast<ParallelGateway>(kv_pair.second)) {
      active_inputs_by_child[kv_pair.first] =
          kv_pair.second->ListActiveInputPorts();
    }
  }
  return active_inputs_by_child;
}

void Process::TriggerInputsOnChild(
    const std::string &child_uuid,
    const std::vector<std::string> &input_port_uuids) {

  for (const auto &input_port_uuid : input_port_uuids) {
    try {
      this->process_definition_.at(child_uuid)
          ->triggerInputPort(input_port_uuid);
    } catch (const std::out_of_range & /* e */) {
      continue;
    }
  }
}

} /* namespace trace */
