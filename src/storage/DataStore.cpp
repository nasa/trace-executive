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

#include "trace/log/macros.hpp"
#include <trace/storage/DataStore.hpp>

#include <exception>
#include <regex>
#include <type_traits>

namespace trace {

DataStore::DataStore() {
  // database will be kept in memory only
}

DataStore &DataStore::instance() {
  static DataStore instance;
  return instance;
}

void DataStore::SerializeTo(Snapshot &snapshot) const {
  snapshot.SaveDataStore(this->db_);
}

void DataStore::DeserializeFrom(const Snapshot &snapshot) {
  for (const auto &kv_pair : snapshot.FetchSavedDataStore()) {
    this->db_[kv_pair.first] = kv_pair.second;
  }
}

std::string DataStore::FromExpression(const std::string &expression) {

  const std::regex txt_regex("(\\$\\{.*?\\})");
  std::smatch txt_match;

  std::string magic_value = expression;

  std::string::const_iterator start(expression.cbegin());

  size_t index = 0;

  while (std::regex_search(start, expression.cend(), txt_match, txt_regex)) {

    TRACE_LOG_INFO("DataStore", "match_size: " << txt_match.size());
    TRACE_LOG_INFO("DataStore", "prefix: " << txt_match.prefix().str());
    TRACE_LOG_INFO("DataStore", "suffix: " << txt_match.suffix().str());

    TRACE_LOG_INFO("DataStore", "lookup: " << txt_match[0]);
    start = txt_match.suffix().first;

    // 1. Strip magic word, format ${key,default_value} or ${key}
    std::string magic_word = txt_match[0].str();
    std::string magic_key = magic_word.substr(2, magic_word.length() - 3);

    // 2. Figure out if "key" or "key,default"
    auto sub_index = magic_key.find_first_of(",");
    std::string key = magic_key.substr(0, sub_index);
    std::string default_value = "";

    if (sub_index != std::string::npos) {
      default_value = magic_key.substr(sub_index + 1);
    }

    // 3. Look-up magic_value in DataStore, as value
    std::string value = "";
    TRACE_LOG_INFO("DataStore",
                   "Searching data store for key (" << key << ").");
    try {
      value = DataStore::instance().get_value_by_key<std::string>(key);
      TRACE_LOG_INFO("DataStore", "Resolved (" << key << ", " << value << ").");
    } catch (const std::out_of_range &e) {
      if (sub_index == std::string::npos) {
        TRACE_LOG_ERROR("DataStore", "Unable to resolve (" << key << ")");
        throw std::runtime_error("DataStore lookup failed for key (" + key +
                                 ") "
                                 "and no default value was specified.");
      } else {
        value = default_value;
        TRACE_LOG_INFO("DataStore", "Resolved using default value ("
                                        << key << ", " << default_value
                                        << ").");
      }
    }

    // 4. Replace magic_word with value from DataStore
    index = magic_value.find(magic_word, index);
    magic_value.replace(index, magic_word.length(), value);
    index += value.length();
  }

  TRACE_LOG_INFO("DataStore", "final magic value is (" << magic_value << ")");
  return magic_value;
}

} // namespace trace
