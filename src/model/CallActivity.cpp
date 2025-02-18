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

#include <trace/model/CallActivity.hpp>

namespace trace {

CallActivity::CallActivity(const std::string &uuid, const std::string &name)
    : Activity(uuid, name) {}

CallActivity::CallActivity(const CallActivity &call_activity)
    : Activity(call_activity) {
  call_uuid_ = call_activity.call_uuid_;
}

CallActivity::~CallActivity() {}

std::string CallActivity::getCallUuid() { return call_uuid_; }

void CallActivity::setCallUuid(const std::string &call_uuid) {
  call_uuid_ = call_uuid;
}

void CallActivity::start() {
  connection_ = this->instance_->attachTokenTerminationEventCallback(
      boost::bind(&CallActivity::token_termination_event_callback, this,
                  boost::placeholders::_1, boost::placeholders::_2,
                  boost::placeholders::_3));
  Activity::start();
  this->instance_->emitNewToken(call_uuid_);
}

void CallActivity::resume() {
  connection_ = this->instance_->attachTokenTerminationEventCallback(
      boost::bind(&CallActivity::token_termination_event_callback, this,
                  boost::placeholders::_1, boost::placeholders::_2,
                  boost::placeholders::_3));
  Activity::resume();
}

void CallActivity::token_termination_event_callback(
    const std::string &token_uuid, const std::string &node_uuid,
    const Types::ExitStatusCode &status) {
  if (call_uuid_.compare(node_uuid) == 0) {
    TRACE_LOG_INFO(uuid(), "Received a TokenTerminationNotification ("
                               << token_uuid << ") from " << node_uuid
                               << " with status (" << status << ").");

    switch (status) {
    case Types::ExitStatusCode::FALSE_ALARM:
      throw std::runtime_error("A false alarm should never happen!");
      break;
    case Types::ExitStatusCode::OK:
      TRACE_LOG_INFO(uuid(), "Call activity terminated successfully.");
      break;
    case Types::ExitStatusCode::EXCEPTION:
      TRACE_LOG_INFO(uuid(), "Call activity terminated with an exception.");
      break;
    }

    {
      std::lock_guard<std::mutex> lock(is_active_mutex_);
      called_token_status_ = status;
    }
    external_termination_signaled_.notify_all();
  }
}

void CallActivity::waitForTokenTerminationNotification() {}

void CallActivity::interrupt() {
  TRACE_LOG_INFO(uuid(), "This call activity is being interrupted.");
  is_interrupted_ = true;
  // REVIEW: Assumption that call activities only point to processes
  auto process = this->instance_->activeProcessByUuid(call_uuid_);
  TRACE_LOG_INFO(uuid(), "Interrupting called activity ("
                             << process->uuid() << ", " << process->name()
                             << ")");
  process->interrupt();
}

void CallActivity::terminate() {
  TRACE_LOG_INFO(uuid(), "This call activity is being terminated.");
  // REVIEW: Assumption that call activities only point to processes
  auto process = this->instance_->activeProcessByUuid(call_uuid_);
  process->terminate();
  Activity::terminate();
}

Outcome CallActivity::activity() {

  TRACE_LOG_INFO(uuid(), "I promise that I, " << name() << " (" << uuid()
                                              << "), am working on ("
                                              << call_uuid_ << ")!");

  std::unique_lock<std::mutex> lock(is_active_mutex_);
  external_termination_signaled_.wait(lock);
  auto rc = called_token_status_;
  auto boundary_event_uuid = interrupting_boundary_event_uuid_;
  lock.unlock();

  if (is_interrupted_) {
    auto outcome =
        Outcome(StatusCode::INTERRUPTED, "Called activity was interrupted.");
    outcome.add_result("boundary_event_uuid", boundary_event_uuid);
    return outcome;
  }

  if (rc == Types::ExitStatusCode::FALSE_ALARM) {
    return Outcome(StatusCode::CANCELLED, "Called activity was cancelled.");
  } else if (rc == Types::ExitStatusCode::OK) {
    return Outcome(StatusCode::OK, "Called activity completed successfully.");
  } else { // if (rc == Types::ExitStatusCode::EXCEPTION) {
    return Outcome(StatusCode::ERROR,
                   "Called activity terminated unsuccessfully.");
  }
}

void CallActivity::cleanup() {
  connection_.disconnect();
  Activity::cleanup();
}

} /* namespace trace */
