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

#ifndef TRACE_MODEL_H_
#define TRACE_MODEL_H_

// TRACE
#include "trace/Types.hpp"
#include "trace/model/Process.hpp"
#include "trace/token.h"

// Boost
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
#include <boost/signals2/connection.hpp>
#include <boost/signals2/signal.hpp>

// C++11
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace trace {

class Token;

class Model {

public:
  std::shared_ptr<Process> getProcessByUuid(const std::string &uuid);
  std::shared_ptr<ProcessModelNode> getNodeByUuid(const std::string &uuid);
  std::shared_ptr<ProcessModelNode>
  getActiveNodeByUuid(const std::string &uuid);
  std::shared_ptr<Token> getActiveTokenByNodeUuid(const std::string &uuid);

  std::vector<std::shared_ptr<ProcessModelNode>>
  activeNodesByProcessUuid(const std::string &process_uuid);
  std::vector<std::shared_ptr<Token>>
  activeTokensByProcessUuid(const std::string &process_uuid);
  std::shared_ptr<Process> activeProcessByUuid(const std::string &process_uuid);
  std::pair<std::string, std::string>
  getParentInfoByChildUuid(const std::string &child_uuid);

  // std::string getParentProcessUuid(const std::string &node_uuid);

  std::shared_ptr<Token> emitNewToken(const std::string &node_uuid,
                                      bool reanimated = false);
  std::unordered_map<std::string, std::string> getSignalList();

  std::vector<std::string> getActiveNodeUuids();

  void token_termination_event_callback(const std::string &token_uuid,
                                        const std::string &node_uuid,
                                        const Types::ExitStatusCode &status);

  bool hasMissionLiftoff();

  bool isActive();

  void start(const std::string &encoded_snapshot = "");
  void waitForTermination();
  void reset();
  void abort();

  bool validate();

  // void publishMissionStatus();
  // Types::MissionStatus getMissionStatus();

  int activeTokenCountByProcess(const std::string &process_uuid);

  bool initializeFromXml(const std::string &url);
  virtual ~Model(){};

  boost::signals2::connection attachTokenTerminationEventCallback(
      boost::function<void(const std::string &, const std::string &,
                           const Types::ExitStatusCode &)>
          callback);
  void notifyOfTokenTermination(const std::string &token_uuid,
                                const std::string &node_uuid,
                                const Types::ExitStatusCode &status);

  bool emitSignal(const std::string &signal_uuid);

  std::string generateEncodedSnapshot();

  void punchIn(const std::string &token_uuid, const std::string &node_uuid);
  void punchOut(const std::string &token_uuid);
  void print_punched_in_tokens();

  std::size_t cleanup_tokens();

  Model();

private:
  std::string bpmn_xml_url_;

  Types::MissionStatus mission_status_;
  std::mutex mission_status_mutex_;

  void setMissionStatus(const Types::MissionStatus &status);
  bool setMissionStatusIf(const Types::MissionStatus &status,
                          const Types::MissionStatus &new_status);
  Types::MissionStatus getMissionStatus();

  boost::signals2::signal<void(const std::string &, const std::string &,
                               const Types::ExitStatusCode &)>
      token_termination_event_signal_;
  boost::signals2::connection token_termination_event_listener_;

  std::string main_token_uuid_;

  std::unordered_map<std::string, std::shared_ptr<Process>> processes_;
  std::unordered_map<std::string, std::string> signals_;
  std::unordered_map<std::string, std::shared_ptr<ModelEvent>> events_;
  std::unordered_map<std::string, std::shared_ptr<Token>> tokens_;
  std::unordered_map<std::string, std::string> token_punch_card_;
  std::mutex process_list_mutex_, signal_list_mutex_, token_list_mutex_,
      token_punch_card_mutex_;

  static const std::string TAG;
};

} // namespace trace

#endif /* TRACE_MODEL_H_ */
