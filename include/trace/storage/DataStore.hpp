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

#pragma once

#include "trace/Snapshot.hpp"

// C++
#include <memory>
#include <mutex>
#include <thread>

#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace trace {

class DataStore {

public:
  static DataStore &instance();

  template <class V>
  void set_value_by_key(const std::string &key, const V &value);

  template <class V> V get_value_by_key(const std::string &key);

  void SerializeTo(Snapshot &snapshot) const;
  void DeserializeFrom(const Snapshot &snapshot);

  std::string FromExpression(const std::string &expression);

protected:
  DataStore();
  virtual ~DataStore(){};

  // broken, but cool idea using variadic templates
  // template<class...T>
  // std::vector<Record> query(const std::string &sql_expression,
  // T...bind_values) {
  //   std::lock_guard<std::mutex> lock(db_mutex_);
  //   Query q = db_->query(sql_expression);
  //   auto bind_value_list = std::tie(bind_values...);
  //   for (int i=0; i<sizeof...(bind_values); i++) {
  //     q.bind(std::get<i>(bind_value_list));
  //   }
  //   return q.exec();
  // }

private:
  std::unordered_map<std::string, std::string> db_;
  std::mutex db_mutex_;
};

} // namespace trace

#include <trace/storage/DataStore.tpp>