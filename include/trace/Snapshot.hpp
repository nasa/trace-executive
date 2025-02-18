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

#ifndef TRACE_SNAPSHOT_HPP_
#define TRACE_SNAPSHOT_HPP_

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <fstream>
#include <sstream>
#include <string>

#include <map>
#include <unordered_map>

namespace trace {

class Snapshot {

  friend class boost::serialization::access;
  friend std::ostream &operator<<(std::ostream &os, const Snapshot &snapshot);

  std::map<std::string, std::string> active_token_element_uuids_;
  std::string model_url_;
  std::map<std::string, std::string> data_store_;
  std::map<std::string, std::vector<std::string>>
      parallel_gateway_active_inputs_;

  template <class Archive>
  void serialize(Archive &ar, const unsigned int /* version */) {
    // ar & variable;
    // ...
    ar &active_token_element_uuids_;
    ar &model_url_;
    ar &parallel_gateway_active_inputs_;
    ar &data_store_;
  }

public:
  Snapshot();
  virtual ~Snapshot() {}

  void addActiveTokenWithElementUuid(const std::string &token_uuid,
                                     const std::string &element_uuid);
  void setModelUrl(const std::string &model_url);
  std::string getModelUrl();

  /**
   * @brief Add an input (by UUID) to the list of active inputs for a particular
   * parallel gateway specified by its UUID.
   *
   * @param parallel_gateway_uuid The UUID of the parallel gateway with the
   * active input.
   * @param active_input_uuid The UUID of the active input to the parallel
   * gateway.
   */
  void AddParallelGatewayActiveInput(const std::string &parallel_gateway_uuid,
                                     const std::string &active_input_uuid);

  /**
   * @brief Fetch all active inputs for the parallel gateway specified by the
   * provided UUID.
   *
   * @param parallel_gateway_uuid The UUID of the parallel gateway.
   * @return std::vector<std::string> All active inputs to the parallel
   * gatewayw.
   */
  std::vector<std::string>
  GetParallelGatewayActiveInputs(const std::string &parallel_gateway_uuid);

  std::map<std::string, std::string> getActiveTokenWithElementUuids();

  /**
   * @brief Insert the data store's internal map into the snapshot.
   *
   * @param data_store_internal_map Data store's internal map to be saved in the
   * snapshot.
   */
  void SaveDataStore(const std::unordered_map<std::string, std::string>
                         &data_store_internal_map);

  /**
   * @brief Fetch the data store's internal map from this snapshot.
   *
   * @return std::unordered_map<std::string, std::string> Data store's internal
   * map saved in the snapshot.
   */
  std::map<std::string, std::string> FetchSavedDataStore() const;
};

} // namespace trace

#endif
