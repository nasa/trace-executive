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

#include "trace/Snapshot.hpp"

namespace trace {

Snapshot::Snapshot() {
  // empty snapshot
}

void Snapshot::addActiveTokenWithElementUuid(const std::string &token_uuid,
                                             const std::string &element_uuid) {
  active_token_element_uuids_[token_uuid] = element_uuid;
}

void Snapshot::setModelUrl(const std::string &model_url) {
  model_url_ = model_url;
}

std::map<std::string, std::string> Snapshot::getActiveTokenWithElementUuids() {
  return active_token_element_uuids_;
}

std::string Snapshot::getModelUrl() { return model_url_; }

void Snapshot::AddParallelGatewayActiveInput(
    const std::string &parallel_gateway_uuid,
    const std::string &active_input_uuid) {
  this->parallel_gateway_active_inputs_[parallel_gateway_uuid].push_back(
      active_input_uuid);
}

std::vector<std::string> Snapshot::GetParallelGatewayActiveInputs(
    const std::string &parallel_gateway_uuid) {
  std::vector<std::string> active_input_uuids;
  try {
    active_input_uuids =
        this->parallel_gateway_active_inputs_.at(parallel_gateway_uuid);
  } catch (const std::out_of_range & /* e */) {
    // no-op
  }
  return active_input_uuids;
}

void Snapshot::SaveDataStore(const std::unordered_map<std::string, std::string>
                                 &data_store_internal_map) {
  this->data_store_.clear();
  for (const auto &kv_pair : data_store_internal_map) {
    this->data_store_[kv_pair.first] = kv_pair.second;
  }
}

std::map<std::string, std::string> Snapshot::FetchSavedDataStore() const {
  return this->data_store_;
}

} // namespace trace
