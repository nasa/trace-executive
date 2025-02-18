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

#include "trace/SnapshotProcessor.hpp"

namespace trace {

Snapshot SnapshotProcessor::decode(const std::string &encoded_snapshot) {
  std::stringstream iss(encoded_snapshot);
  Snapshot snapshot;
  {
    boost::archive::text_iarchive ia(iss);
    ia >> snapshot;
  }
  return snapshot;
}

std::string SnapshotProcessor::encode(const Snapshot &snapshot) {
  std::stringstream oss;
  {
    boost::archive::text_oarchive oa(oss);
    oa << snapshot;
  }
  return oss.str();
}

} // namespace trace
