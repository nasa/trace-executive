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

#ifndef TRACE_CONTROL_MISSION_CONTROL_H_
#define TRACE_CONTROL_MISSION_CONTROL_H_

#include "trace/model.h"

#include <string>
#include <vector>

namespace trace {
namespace control {

/**
 * @brief Mission control is responsible for controlling the state of the
 * execution at the mission level--that is to say that one can start and abort
 * missions, as well as, suspend and resume these missions.
 *
 */
class MissionControl {

public:
  /**
   * @brief Construct a new Mission Control object without any connectors to
   * load and initialize.
   *
   */
  MissionControl() noexcept;

  /**
   * @brief Construct a new Mission Control object and load a connector.
   *
   * @param base_connector_id Identifier for the connector in control.
   */
  MissionControl(const std::string &base_connector_id);

  /**
   * @brief Destroy the Mission Control object.
   *
   */
  virtual ~MissionControl();

  /**
   * @brief Start the mission using a specific BPMN mission model located on the
   * filesystem at the provided URL. Does not block.
   *
   * @param mission_model_url The path to the BPMN mission model on the file
   * system.
   * @return A boolean indicating whether starting the mission was successful or
   * not.
   */
  bool start(const std::string &mission_model_url);

  /**
   * @brief Aborts the mission and cleans up its state(s).
   *
   */
  void abort();

  /**
   * @brief Suspends the mission and saves a snapshot of the execution state to
   * a file on the file system. If unable to write to file, then abort anyways.
   *
   * @param snapshot_url File system location where the execution state snapshot
   * will be written.
   */
  void SuspendToUrl(const std::string &snapshot_url);

  /**
   * @brief Suspend the mission and saves a snapshot of the execution state ot
   * a string.
   *
   * @return The byte string representing the executive's state at time of
   * suspension
   */
  std::string SuspendToByteString();

  /**
   * @brief Returns the snapshot as an encoded byte string.
   *
   * @return The byte string representing the executive's state at this point in
   * time.
   */
  std::string FetchSnapshotByteString() const;

  /**
   * @brief Resume the mission using a snapshot from the file system. Does not
   * block.
   *
   * @param snapshot The snapshot containing all information needed to resume
   * the mission in its previous state.
   */
  bool ResumeFromUrl(const std::string &snapshot_url);

  /**
   * @brief Resume the mission using a snapshot in byte form. Does not block.
   *
   * @param snapshot The snapshot containing all information needed to resume
   * the mission in its previous state.
   */
  bool ResumeFromByteString(const std::string &snapshot_bytes);

  /**
   * @brief Wait (block) for the mission to conclude successfully or not.
   *
   */
  void wait();

  /**
   * @brief Returns a list of currently active flow nodes in the BPMN model,
   * i.e., flow node elements with a token.
   *
   * @return A list of the currently active flow nodes.
   */
  std::vector<std::string> FetchActiveFlowNodeUuids() const;

  /**
   * @brief Checks whether the mission is currently running.
   *
   * @return A boolean that is true if the mission is running and false
   * otherwise.
   */
  bool IsMissionInProgress() const;

private:
  static const std::string TAG;

  std::unique_ptr<Model> model_;
};

} // namespace control
} // namespace trace

#endif