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

#include "trace/control/mission_control.h"

#include "trace/Snapshot.hpp"
#include "trace/SnapshotProcessor.hpp"
#include "trace/connector/ConnectorManager.hpp"
#include "trace/log/connector_output_target.hpp"
#include "trace/log/macros.hpp"
#include "trace/model.h"

#include <fstream>
#include <sstream>

namespace trace {
namespace control {

const std::string MissionControl::TAG = "mission_control";

MissionControl::MissionControl() noexcept {
  this->model_ = std::unique_ptr<Model>(new Model());
}

MissionControl::MissionControl(const std::string &base_connector_id) {
  ConnectorManager::instance().load(
      "libtrace_" + base_connector_id + "_connector.so", true);

  trace::log::_global_logger.addTarget(
      "connector", std::make_shared<trace::log::ConnectorOutputTarget>(
                       "trace_" + base_connector_id + "_connector"));
  // Console::instance().attach_connector("trace_" + base_connector_id +
  //                                      "_connector");

  this->model_ = std::unique_ptr<Model>(new Model());
}

MissionControl::~MissionControl() {
  // abort();

  // Console::instance().detach_connector();
  trace::log::_global_logger.removeTarget("connector");
  ConnectorManager::instance().unload_all();

  this->model_.reset();
}

bool MissionControl::start(const std::string &mission_model_url) {

  if (this->model_->hasMissionLiftoff()) {
    TRACE_LOG_WARN(TAG,
                   "Active mission is aborted before starting new mission.");
    abort();
  }

  bool ok = this->model_->initializeFromXml(mission_model_url);
  if (!ok) {
    TRACE_LOG_FATAL(TAG, "Unable to execute mission model. Exiting.");
    return false;
  }

  ok = this->model_->validate();

  if (!ok) {
    TRACE_LOG_FATAL(
        TAG, "Unable to validate mission model with connector. Exiting.");
    return false;
  }

  // Timeline::instance().start_mission();
  this->model_->start();
  return true;
}

void MissionControl::abort() {
  TRACE_LOG_INFO(TAG, "Aborting mission...");
  this->model_->abort();
  this->model_->reset();
  TRACE_LOG_INFO(TAG, "...aborted.");
  // Timeline::instance().end_mission();
  // Timeline::instance().write_to_file();
}

void MissionControl::wait() {
  TRACE_LOG_INFO(TAG, "Awaiting termination of mission...");
  this->model_->waitForTermination();
  TRACE_LOG_INFO(TAG, "...wait is over.");
}

void MissionControl::SuspendToUrl(const std::string &snapshot_url) {
  std::ofstream snapshot(snapshot_url, std::ios::binary);
  if (snapshot.is_open()) {
    TRACE_LOG_INFO(TAG, "Saving snapshot to file system.");
    snapshot << this->model_->generateEncodedSnapshot();
  } else {
    TRACE_LOG_WARN(TAG, "Unable to save snapshot. Aborting anyways.");
  }
  abort();
}

std::string MissionControl::SuspendToByteString() {
  auto snapshot_byte_string = this->model_->generateEncodedSnapshot();
  abort();
  return snapshot_byte_string;
}

bool MissionControl::ResumeFromUrl(const std::string &snapshot_url) {
  std::ifstream snapshot(snapshot_url, std::ios::binary);

  if (this->model_->hasMissionLiftoff()) {
    TRACE_LOG_WARN(TAG,
                   "Active mission is aborted before starting new mission.");
    abort();
  }

  if (!snapshot.is_open()) {
    TRACE_LOG_ERROR(TAG,
                    "Unable to load snapshot from file system for resume.");
    return false;
  }

  std::stringstream ss;
  ss << snapshot.rdbuf();

  return ResumeFromByteString(ss.str());
}

bool MissionControl::ResumeFromByteString(const std::string &snapshot_bytes) {

  auto decoded_snapshot = SnapshotProcessor::decode(snapshot_bytes);

  bool ok = this->model_->initializeFromXml(decoded_snapshot.getModelUrl());
  if (!ok) {
    TRACE_LOG_FATAL(TAG, "Unable to execute mission model. Exiting.");
    return false;
  }

  ok = this->model_->validate();

  if (!ok) {
    TRACE_LOG_FATAL(
        TAG, "Unable to validate mission model with connector. Exiting.");
    return false;
  }

  // Timeline::instance().start_mission();
  this->model_->start(snapshot_bytes);
  return true;
}

std::string MissionControl::FetchSnapshotByteString() const {
  return this->model_->generateEncodedSnapshot();
}

std::vector<std::string> MissionControl::FetchActiveFlowNodeUuids() const {
  return this->model_->getActiveNodeUuids();
}

bool MissionControl::IsMissionInProgress() const {
  return this->model_->hasMissionLiftoff();
}

} // namespace control
} // namespace trace