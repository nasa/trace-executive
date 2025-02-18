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

#include <trace/model/extension/ConnectorServiceTask.hpp>

// #include "trace/timeline.h"
#include <trace/connector/ConnectorManager.hpp>

#include "trace/storage/DataStore.hpp"

namespace trace {

ConnectorServiceTask::ConnectorServiceTask(const std::string &uuid,
                                           const std::string &name,
                                           const std::string &connector_id)
    : ServiceTask(uuid, name),
      connector_activity_outcome_(StatusCode::CANCELLED,
                                  "Connector activity was cancelled."),
      is_external_terminate_signaled_(false), is_destructing_(false) {
  connector_id_ = connector_id;
}

ConnectorServiceTask::ConnectorServiceTask(
    const ConnectorServiceTask &connector_service_task)
    : ServiceTask(connector_service_task) {
  is_external_terminate_signaled_ = false;
  is_destructing_ = false;

  connector_id_ = connector_service_task.connector_id_;
  connector_activity_outcome_ =
      connector_service_task.connector_activity_outcome_;
}

ConnectorServiceTask::~ConnectorServiceTask() {
  is_destructing_ = true;
  external_termination_signaled_for_real_ = true;
  external_termination_signaled_.notify_all();
}

void ConnectorServiceTask::resume() { start(); }

void ConnectorServiceTask::start() {
  // ConnectorManager::instance().connector(connector_id_)->add_connection(uuid(),
  // data_input_store_,
  // std::bind(&ConnectorServiceTask::connector_activity_outcome_callback, this,
  // std::placeholders::_1));
  wait_for_terminate_signal_future_ =
      std::async(std::launch::async,
                 &ConnectorServiceTask::wait_for_terminate_signal, this);

  bool success = ConnectorManager::instance()
                     .connector(connector_id_)
                     ->reserve_resource(this->uuid(), retrieveDataInputStore());
  if (!success) {
    throw std::runtime_error("Unable to reserve resource!");
  }
  ServiceTask::start();
}

void ConnectorServiceTask::wait_for_terminate_signal() {
  std::unique_lock<std::mutex> lock(is_active_mutex_);
  while (!external_termination_signaled_for_real_) {
    external_termination_signaled_.wait(lock);
  }
  if (!is_destructing_) {
    is_external_terminate_signaled_ = true;
  }
  lock.unlock();
}

Outcome ConnectorServiceTask::activity() {
  TRACE_LOG_INFO(uuid(), "I promise that I, " << name() << " (" << uuid()
                                              << "), am working on it via "
                                              << connector_id_ << "!");

  // FIXME: Disabled until works with suspend/resume
  // Timeline::instance().start_activity(uuid(), name());

  std::shared_future<Outcome> service_future;
  try {
    // ConnectorManager::instance().connector(connector_id_)->send_command(uuid(),
    // retrieveDataInputStore());
    service_future =
        ConnectorManager::instance()
            .connector(connector_id_)
            ->command_resource(this->uuid(), retrieveDataInputStore());
  } catch (const std::runtime_error &e) {
    // Timeline::instance().end_activity(uuid());
    return Outcome(StatusCode::ERROR, e.what());
  }

  bool outcome_ready = false;
  Outcome outcome;
  do {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::unique_lock<std::mutex> lock(this->external_termination_signal_mutex_);
    // auto status = external_termination_signaled_.wait_for(
    //     lock, std::chrono::milliseconds(100));
    if (!is_external_terminate_signaled_) {
      if (this->is_interrupted_) {
        // we got interrupted, but missed it
        outcome = Outcome(StatusCode::INTERRUPTED,
                          "Connector activity was interrupted.");
        outcome.add_result("boundary_event_uuid",
                           interrupting_boundary_event_uuid_);
        outcome_ready = true;

        ConnectorManager::instance()
            .connector(connector_id_)
            ->abort_command(this->uuid(), retrieveDataInputStore());
        service_future.wait(); // wait on the abort to go through
      } else {
        auto future_status = service_future.wait_for(std::chrono::seconds(0));
        if (future_status == std::future_status::ready) {
          outcome = service_future.get();
          for (auto result : outcome.results()) {
            const std::string key = uuid() + "." + result.first;
            TRACE_LOG_INFO(uuid(),
                           "IN CST.cpp: Adding to data store from result: ("
                               << key << ", " << result.second << ").");
            DataStore::instance().set_value_by_key<std::string>(key,
                                                                result.second);

            // FIXME: Disabled until it works with suspend/resume
            // try {
            //   std::stod(result.second);
            //   Timeline::instance().record_state(result.first, result.second);
            // } catch (...) {
            //   // skip
            //   Timeline::instance().record_text_state(result.first,
            //                                          result.second);
            // }
          }
          outcome_ready = true;
        }
      }
    } else {
      if (is_interrupted_) {
        outcome = Outcome(StatusCode::INTERRUPTED,
                          "Connector activity was interrupted.");
        outcome.add_result("boundary_event_uuid",
                           interrupting_boundary_event_uuid_);
        outcome_ready = true;
      } else {
        outcome =
            Outcome(StatusCode::CANCELLED, "Connector activity was cancelled.");
        outcome_ready = true;
      }
      ConnectorManager::instance()
          .connector(connector_id_)
          ->abort_command(this->uuid(), retrieveDataInputStore());
      service_future.wait(); // wait on the abort to go through
    }
  } while (!outcome_ready);

  // FIXME: Disabled until works with suspend/resume
  // Timeline::instance().end_activity(uuid());
  return outcome;
}

void ConnectorServiceTask::cleanup() {
  ConnectorManager::instance()
      .connector(connector_id_)
      ->release_resource(uuid(), retrieveDataInputStore());
  ServiceTask::cleanup();
}

} // namespace trace
