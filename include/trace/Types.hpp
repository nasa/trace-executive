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

#ifndef TRACE_TYPES_HPP_
#define TRACE_TYPES_HPP_

namespace trace {

class Types {

public:
  enum TaskType : int { UNDEFINED, SCRIPT, SERVICE, USER };

  // TODO: deprecating in favor of StatusCode
  enum ExitStatusCode : int {
    FALSE_ALARM, // FIXME: enum types should be prefixed
    OK,
    EXCEPTION
  };

  enum MissionStatus : int {
    UNINITIALIZED,
    READY,
    IN_PROGRESS,
    SUCCESS,
    FAILED,
    ABORTED
  };

  enum MissionAction : int {
    ACTION_LOAD_MISSION,
    ACTION_START_MISSION,
    ATCTION_ABORT_MISSION
  };

  enum ImplementationType : char { NATIVE = 'N', CUSTOM = 'C' };
};

} // namespace trace

#endif
