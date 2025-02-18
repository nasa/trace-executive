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

// TRACE
#include "trace/model.h"

#include "trace/SnapshotProcessor.hpp"
#include "trace/bpmn/Parser.hpp"
#include "trace/connector/ConnectorManager.hpp"
#include "trace/storage/DataStore.hpp"

#include <chrono>
#include <thread>

namespace trace {

const std::string Model::TAG = "model";

Model::Model() : mission_status_(Types::MissionStatus::UNINITIALIZED) {}

bool Model::initializeFromXml(const std::string &url) {
  std::lock_guard<std::mutex> lock(this->mission_status_mutex_);
  if (this->mission_status_ == Types::MissionStatus::UNINITIALIZED) {
    TRACE_LOG_INFO(TAG, "Parsing mission file: ");
    try {
      auto outcome = Parser().extractModelFromXml(url, processes_, events_);
      bpmn_xml_url_ = url;

      if (outcome.ok()) {
        mission_status_ = Types::MissionStatus::READY;
        TRACE_LOG_INFO(TAG, "Mission model is successfully loaded.");
        return true;
      } else {
        TRACE_LOG_ERROR(TAG, outcome.details());
        // return false;
      }
    } catch (const std::runtime_error &e) {
      TRACE_LOG_ERROR(TAG, e.what());
      // return false;
    }
  } else {
    TRACE_LOG_ERROR(
        TAG,
        "Cannot reinitialize the executive with a new mission at this point.");
  }
  return false;
}

void Model::reset() {
  std::lock_guard<std::mutex> lock(this->mission_status_mutex_);
  if (mission_status_ != Types::MissionStatus::IN_PROGRESS) {
    {
      // TRACE_LOG_INFO(TAG, "Acquiring token_list_mutex...");
      std::lock_guard<std::mutex> lock(token_list_mutex_);
      // TRACE_LOG_INFO(TAG, "...acquired.");
      tokens_.clear();
      processes_.clear();
      events_.clear();
    }
    mission_status_ = Types::MissionStatus::UNINITIALIZED;
  } else {
    TRACE_LOG_WARN(TAG, "Unable to reset mission while in progress.");
  }
}

bool Model::validate() {

  std::lock_guard<std::mutex> lock(this->process_list_mutex_);

  for (const auto &process : this->processes_) {
    TRACE_LOG_INFO(TAG, "Validating implemented nodes in process ("
                            << process.first << "):");
    auto is_valid = process.second->validate();
    if (is_valid == false) {
      return false;
    }
  }
  return true;
}

std::string Model::generateEncodedSnapshot() {
  Snapshot snapshot;

  // 1. Save out all tokens
  {
    std::string token_uuid, element_uuid;

    // TRACE_LOG_INFO(TAG, "Acquiring token_list_mutex...");
    std::lock_guard<std::mutex> lock(token_list_mutex_);
    // TRACE_LOG_INFO(TAG, "...acquired.");

    for (auto kv_pair : tokens_) {
      token_uuid = kv_pair.first;

      auto token = kv_pair.second;
      element_uuid = token->getCurrentNodeUuid();

      if (element_uuid == "NODE_WITHOUT_VALID_UUID") {
        // skip
        continue;
      }
      snapshot.addActiveTokenWithElementUuid(token_uuid, element_uuid);
    }
  }

  // 2. Save out mission model URL
  snapshot.setModelUrl(bpmn_xml_url_);

  // 3. Save out all parallel gateway active inputs
  {
    for (const auto &kv_pair : this->processes_) {
      for (const auto &child_kv_pair :
           kv_pair.second->PeekActiveInputsForEachChild()) {
        for (const auto &input_port : child_kv_pair.second) {
          snapshot.AddParallelGatewayActiveInput(child_kv_pair.first,
                                                 input_port);
        }
      }
    }
  }

  // 4. Save out data store.
  DataStore::instance().SerializeTo(snapshot);

  return SnapshotProcessor::encode(snapshot);
}

void Model::abort() {
  if (setMissionStatusIf(Types::MissionStatus::IN_PROGRESS,
                         Types::MissionStatus::ABORTED)) {
    std::shared_ptr<Token> main_token = nullptr;
    {
      // TRACE_LOG_INFO(TAG, "Acquiring token_list_mutex...");
      std::lock_guard<std::mutex> lock(token_list_mutex_);
      // TRACE_LOG_INFO(TAG, "...acquired.");
      main_token = tokens_[main_token_uuid_];
    }
    TRACE_LOG_INFO(TAG, "Released token_list_mutex.");

    TRACE_LOG_INFO(TAG, "Preparing to terminate " << main_token_uuid_);
    if (main_token != nullptr) {
      main_token->terminate();
    }
    TRACE_LOG_INFO(TAG, "terminated " << main_token_uuid_);
    // TRACE_LOG_INFO(TAG, "Terminated trace, waiting for abort...");
    // waitForTermination();
    // TRACE_LOG_INFO(TAG, "aborted.");
  } else {
    TRACE_LOG_WARN(
        TAG, "Unable to abort mission while aborting or in aborted state.");
  }
}

std::unordered_map<std::string, std::string> Model::getSignalList() {
  std::lock_guard<std::mutex> lock(signal_list_mutex_);
  return signals_;
}

Types::MissionStatus Model::getMissionStatus() {
  std::lock_guard<std::mutex> lock(this->mission_status_mutex_);
  return mission_status_;
}

bool Model::setMissionStatusIf(const Types::MissionStatus &status,
                               const Types::MissionStatus &new_status) {
  std::lock_guard<std::mutex> lock(this->mission_status_mutex_);
  if (this->mission_status_ == status) {
    this->mission_status_ = new_status;
    return true;
  }
  return false;
}

void Model::setMissionStatus(const Types::MissionStatus &status) {
  std::lock_guard<std::mutex> lock(this->mission_status_mutex_);
  this->mission_status_ = status;
}

void Model::token_termination_event_callback(
    const std::string &token_uuid, const std::string &node_uuid,
    const Types::ExitStatusCode &status) {
  TRACE_LOG_INFO(TAG, "Received notification on termination of "
                          << token_uuid << " after " << node_uuid
                          << " exited with status (" << status << ").");

  TRACE_LOG_INFO(TAG, "Punching out " << token_uuid);
  punchOut(token_uuid);

  if (token_uuid == main_token_uuid_) {
    TRACE_LOG_INFO(TAG, "Mission ended.");
    switch (status) {
    case Types::ExitStatusCode::OK:
      setMissionStatusIf(Types::MissionStatus::IN_PROGRESS,
                         Types::MissionStatus::SUCCESS);
      break;
    case Types::ExitStatusCode::EXCEPTION:
      setMissionStatusIf(Types::MissionStatus::IN_PROGRESS,
                         Types::MissionStatus::FAILED);
      break;
    default:
      break;
    }
    token_termination_event_listener_.disconnect();
  }
}

bool Model::isActive() {
  TRACE_LOG_DEBUG(TAG, "Acquiring token_list_mutex...");
  std::lock_guard<std::mutex> lock(token_list_mutex_);
  TRACE_LOG_DEBUG(TAG, "...acquired.");
  TRACE_LOG_DEBUG(TAG, tokens_.size() << " tokens still active.");
  for (auto token : tokens_) {
    TRACE_LOG_DEBUG(TAG, "\t" << token.first << ", "
                              << token.second->getCurrentNodeUuid());
  }
  if (tokens_.size() > 0) {
    for (auto token : tokens_) {
      // TRACE_LOG_INFO(TAG, "\t" << token.first << ", "
      //  << token.second->getCurrentNodeUuid());
      if (!token.second->isTerminated()) {
        return true;
      }
    }
    // return true; // (tokens_[main_token_uuid_]->isActive());
  }
  return false;
}

void Model::start(const std::string &encoded_snapshot) {
  std::lock_guard<std::mutex> lock(this->mission_status_mutex_);
  if (mission_status_ == Types::MissionStatus::READY) {
    std::string main_process_uuid =
        "Process_1"; // FIXME: hardcoded requirement that Process_1 is the
                     // initial process

    token_termination_event_listener_ = attachTokenTerminationEventCallback(
        boost::bind(&Model::token_termination_event_callback, this,
                    boost::placeholders::_1, boost::placeholders::_2,
                    boost::placeholders::_3));

    if (encoded_snapshot.empty()) {
      main_token_uuid_ = emitNewToken(main_process_uuid)->getUuid();
    } else {
      Snapshot snapshot = SnapshotProcessor::decode(encoded_snapshot);

      ///
      for (const auto &kv_pair : this->processes_) {
        for (const auto &parallel_gateway_uuid :
             kv_pair.second->FetchParallelGatewayUuids()) {
          kv_pair.second->TriggerInputsOnChild(
              parallel_gateway_uuid,
              snapshot.GetParallelGatewayActiveInputs(parallel_gateway_uuid));
        }
      }

      DataStore::instance().DeserializeFrom(snapshot);
      ///

      for (auto te : snapshot.getActiveTokenWithElementUuids()) {
        if (te.second == main_process_uuid) {
          main_token_uuid_ = emitNewToken(main_process_uuid, true)->getUuid();
        } else {
          emitNewToken(te.second, true);
        }
      }
    }

    TRACE_LOG_INFO(TAG, "Mission has been started.");
    mission_status_ = Types::MissionStatus::IN_PROGRESS;
  } else {
    TRACE_LOG_ERROR(
        TAG,
        "The executive is not able start the mission from its current state.");
  }
}

bool Model::hasMissionLiftoff() {
  return (mission_status_ == Types::MissionStatus::IN_PROGRESS);
}

std::shared_ptr<Process> Model::getProcessByUuid(const std::string &uuid) {
  return processes_[uuid];
}

std::shared_ptr<ProcessModelNode>
Model::getNodeByUuid(const std::string &uuid) {
  for (auto kv : processes_) {
    if (kv.first == uuid) {
      TRACE_LOG_INFO(TAG, uuid << " is a process.");
      return kv.second;
    } else {
      auto node = kv.second->getNodeByUuid(uuid);
      if (node != nullptr) {
        return node;
      }
    }
  }
  return nullptr;
}

std::pair<std::string, std::string>
Model::getParentInfoByChildUuid(const std::string &child_uuid) {
  for (auto &kv : this->processes_) {
    if (kv.second->getNodeByUuid(child_uuid) != nullptr) {
      return std::make_pair(kv.first, kv.second->name());
    }
  }
  return std::make_pair("default", "default");
}

std::shared_ptr<ProcessModelNode>
Model::getActiveNodeByUuid(const std::string &uuid) {
  // TRACE_LOG_INFO(TAG, "Acquiring token_list_mutex...");
  std::lock_guard<std::mutex> lock(token_list_mutex_);
  // TRACE_LOG_INFO(TAG, "...acquired.");
  for (auto kv_pair : tokens_) {
    auto token = kv_pair.second;
    if (token->getCurrentNodeUuid() == uuid) {
      return token->getCurrentNode();
    }
  }
  return nullptr;
}

void Model::print_punched_in_tokens() {
  // {
  //   std::lock_guard<std::mutex> lock(token_punch_card_mutex_);
  //   for (auto token : token_punch_card_) {
  //     TRACE_LOG_INFO(TAG, "punched in: (" << token.first << ", " <<
  //     token.second << ")");
  //   }
  // }
  TRACE_LOG_WARN(TAG, ">>>");
  {
    std::lock_guard<std::mutex> lock(token_list_mutex_);
    for (auto token : tokens_) {
      TRACE_LOG_WARN(TAG, "active: (" << token.first << ", "
                                      << token.second->getCurrentNodeUuid()
                                      << ")");
    }
  }
}

std::vector<std::string> Model::getActiveNodeUuids() {
  std::string uuid, name;
  std::vector<std::string> node_uuids;
  // TRACE_LOG_INFO(TAG, "Acquiring token_list_mutex...");
  std::lock_guard<std::mutex> lock(token_list_mutex_);
  // TRACE_LOG_INFO(TAG, "...acquired.");
  for (auto kv_pair : tokens_) {
    auto token = kv_pair.second;
    uuid = token->getCurrentNodeUuid();
    name = token->getCurrentNodeLocalName();

    if (uuid == "NODE_WITHOUT_VALID_UUID") {
      // skip
      continue;
    }
    node_uuids.push_back(uuid + "," + name);
  }
  return node_uuids;
}

std::shared_ptr<Token>
Model::getActiveTokenByNodeUuid(const std::string &uuid) {
  // TRACE_LOG_INFO(TAG, "Acquiring token_list_mutex...");
  std::lock_guard<std::mutex> lock(token_list_mutex_);
  // TRACE_LOG_INFO(TAG, "...acquired.");
  for (auto kv_pair : tokens_) {
    auto token = kv_pair.second;
    if (token->getCurrentNodeUuid() == uuid) {
      return token;
    }
  }
  return nullptr;
}

bool Model::emitSignal(const std::string &signal_uuid) {
  try {
    events_.at(signal_uuid)->trigger();
    return true;
  } catch (std::out_of_range &e) {
    return false;
  }
}

std::shared_ptr<Token> Model::emitNewToken(const std::string &node_uuid,
                                           bool reanimated) {
  // TRACE_LOG_INFO(TAG, "Acquiring token_list_mutex...");
  std::lock_guard<std::mutex> lock(token_list_mutex_);
  // TRACE_LOG_INFO(TAG, "...acquired.");
  std::shared_ptr<Token> token_ptr =
      std::make_shared<Token>(this, node_uuid, reanimated);
  tokens_[token_ptr->getUuid()] = token_ptr;
  return token_ptr;
}

int Model::activeTokenCountByProcess(const std::string &process_uuid) {
  TRACE_LOG_INFO(TAG, "Counting active tokens for (sub)process ("
                          << process_uuid << "):");
  // TRACE_LOG_INFO(TAG, "Acquiring token_list_mutex...");
  std::lock_guard<std::mutex> lock(token_list_mutex_);
  // TRACE_LOG_INFO(TAG, "...acquired.");
  int active_token_count = 0;
  for (auto kv_pair : tokens_) {
    std::shared_ptr<Token> token = kv_pair.second;
    // TRACE_LOG_INFO(TAG, "What is the current node's uuid?");
    std::string node_uuid = token->getCurrentNodeUuid();
    // std::string node_uuid = token_punch_card_.at(kv_pair.first);
    // TRACE_LOG_INFO(TAG, "It is (" << node_uuid << ").");
    if (processes_[process_uuid]->getNodeByUuid(node_uuid) != nullptr &&
        !token->isTerminated()) { // token->isActive()) { //} &&
                                  // !token->isOverwatch()) {
      TRACE_LOG_INFO(TAG, token->getUuid() << "is an active token on ("
                                           << token->getCurrentNodeUuid()
                                           << "), included in count for ["
                                           << process_uuid << "].");
      active_token_count++;
    }
  }
  TRACE_LOG_INFO(TAG, "All active tokens on children of (sub)process ("
                          << process_uuid << ") accounted for.");
  return active_token_count;
}

std::vector<std::shared_ptr<Token>>
Model::activeTokensByProcessUuid(const std::string &process_uuid) {
  TRACE_LOG_INFO(TAG, "Listing active tokens for (sub)process (" << process_uuid
                                                                 << "):");
  // TRACE_LOG_INFO(TAG, "Acquiring token_list_mutex...");
  std::lock_guard<std::mutex> lock(token_list_mutex_);
  // TRACE_LOG_INFO(TAG, "...acquired.");
  std::vector<std::shared_ptr<Token>> children;
  for (auto kv_pair : tokens_) {
    std::shared_ptr<Token> token = kv_pair.second;
    // TRACE_LOG_INFO(TAG, "What is the current node's uuid?");
    std::string node_uuid = token->getCurrentNodeUuid();
    // std::string node_uuid = token_punch_card_.at(kv_pair.first);
    // TRACE_LOG_INFO(TAG, "It is (" << node_uuid << ").");
    if (processes_[process_uuid]->getNodeByUuid(node_uuid) != nullptr &&
        !token->isTerminated()) { // token->isActive()) { //} &&
                                  // !token->isOverwatch()) {
      TRACE_LOG_INFO(TAG, token->getUuid() << "is an active token on ("
                                           << token->getCurrentNodeUuid()
                                           << "), included in list for ["
                                           << process_uuid << "].");
      children.push_back(token);
    }
  }
  TRACE_LOG_INFO(TAG, "All active tokens on children of (sub)process ("
                          << process_uuid << ") accounted for.");
  return children;

  // // TRACE_LOG_INFO(TAG, "Acquiring token_list_mutex...");
  // std::lock_guard<std::mutex> lock(token_list_mutex_);
  // // TRACE_LOG_INFO(TAG, "...acquired.");
  // std::vector<std::shared_ptr<Token>> children;
  // for (auto kv_pair : tokens_) {
  //   std::shared_ptr<Token> token = kv_pair.second;
  //   try {
  //     std::string node_uuid = token_punch_card_.at(kv_pair.first);
  //     if (processes_[process_uuid]->getNodeByUuid(node_uuid) != nullptr &&
  //         !token->isTerminated()) { // isActive()) { // &&
  //                                   // !token->isOverwatch()) {
  //       children.push_back(token);
  //     }
  //   } catch (const std::out_of_range & /*e*/) {
  //     //
  //   }
  // }
  // return children;
}

std::shared_ptr<Process>
Model::activeProcessByUuid(const std::string &process_uuid) {
  std::lock_guard<std::mutex> lock(token_list_mutex_);
  for (auto kv_pair : tokens_) {
    auto token = kv_pair.second;
    if (!token->isTerminated() &&
        token->getCurrentNodeUuid().compare(process_uuid) == 0) {
      return std::static_pointer_cast<Process>(token->getCurrentNode());
    }
  }
  return nullptr;
}

void Model::punchIn(const std::string &token_uuid,
                    const std::string &node_uuid) {
  std::lock_guard<std::mutex> lock(token_punch_card_mutex_);
  token_punch_card_[token_uuid] = node_uuid;
  TRACE_LOG_INFO(TAG, "Punched in " << token_uuid << " to handle node "
                                    << node_uuid);
  TRACE_LOG_INFO(TAG, "Punch count : " << token_punch_card_.size());
}

void Model::punchOut(const std::string &token_uuid) {
  std::lock_guard<std::mutex> lock(token_punch_card_mutex_);
  // auto count = token_punch_card_.erase(token_uuid);
  // TRACE_LOG_INFO(TAG, "Punched out " << token_uuid << " (" << count << ")
  // times.");
  for (auto it = token_punch_card_.begin(); it != token_punch_card_.end();) {
    if (it->first == token_uuid) {
      TRACE_LOG_INFO(TAG, "Punched out " << it->first << ".");
      it = token_punch_card_.erase(it);
    } else {
      ++it;
    }
  }
  TRACE_LOG_INFO(TAG, "Punch count : " << token_punch_card_.size());
}

std::vector<std::shared_ptr<ProcessModelNode>>
Model::activeNodesByProcessUuid(const std::string &process_uuid) {
  // TRACE_LOG_INFO(TAG, "Acquiring token_list_mutex...");
  std::lock_guard<std::mutex> lock(token_list_mutex_);
  // TRACE_LOG_INFO(TAG, "...acquired.");
  std::vector<std::shared_ptr<ProcessModelNode>> children;
  for (auto kv_pair : tokens_) {
    std::shared_ptr<Token> token = kv_pair.second;
    try {
      std::string node_uuid = token_punch_card_.at(kv_pair.first);
      if (processes_[process_uuid]->getNodeByUuid(node_uuid) != nullptr &&
          !token->isTerminated()) { // isActive()) { // &&
                                    // !token->isOverwatch()) {
        children.push_back(token->getCurrentNode());
      }
    } catch (const std::out_of_range & /*e*/) {
      //
    }
  }
  return children;
}

void Model::waitForTermination() {
  TRACE_LOG_INFO(TAG, "Awaiting termination of mission...");
  TRACE_LOG_INFO(TAG, "is_active? " << std::boolalpha << isActive());

  while (this->isActive()) {

    auto tokens_remaining = cleanup_tokens();
    if (tokens_remaining == 0) {
      break;
    }

    TRACE_LOG_DEBUG(TAG,
                    "This mission is still active. ("
                        << tokens_remaining
                        << ") tokens are still active or busy terminating.");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  TRACE_LOG_INFO(TAG, "(" << cleanup_tokens() << ") tokens remain.");
  TRACE_LOG_INFO(TAG, "This mission is no longer active.");
}

std::size_t Model::cleanup_tokens() {
  std::lock_guard<std::mutex> lock(token_list_mutex_);
  for (auto it = tokens_.begin(); it != tokens_.end();) {
    if (it->second->isTerminated()) {
      TRACE_LOG_DEBUG(TAG, "Handling termination of " << it->first << ".");
      it = tokens_.erase(it);
    } else {
      ++it;
    }
  }
  return this->tokens_.size();
}

boost::signals2::connection Model::attachTokenTerminationEventCallback(
    boost::function<void(const std::string &, const std::string &,
                         const Types::ExitStatusCode &)>
        callback) {
  return token_termination_event_signal_.connect(callback);
}

void Model::notifyOfTokenTermination(const std::string &token_uuid,
                                     const std::string &node_uuid,
                                     const Types::ExitStatusCode &status) {
  token_termination_event_signal_(token_uuid, node_uuid, status);
}

} /* namespace trace */
