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

#ifndef TRACE_SNAPSHOT_PROCESSOR_HPP_
#define TRACE_SNAPSHOT_PROCESSOR_HPP_

#include "trace/Snapshot.hpp"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <string>

namespace trace {

class SnapshotProcessor {

public:
  static std::string encode(const Snapshot &snapshot);
  static Snapshot decode(const std::string &encoded_snapshot);
};

} // namespace trace

#endif
