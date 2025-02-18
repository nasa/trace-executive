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

#ifndef TRACE_MODEL_CALLACTIVITY_HPP_
#define TRACE_MODEL_CALLACTIVITY_HPP_

#include "trace/model.h"
#include <trace/model/Activity.hpp>

#include <trace/Types.hpp>

#include <deque>

namespace trace {

class CallActivity : public Activity {

public:
  CallActivity(const std::string &uuid, const std::string &name);
  CallActivity(const CallActivity &call_activity);
  virtual ~CallActivity();

  COPY_SUPPORT_FUNCTIONS(CallActivity);

  virtual void terminate();

  std::string getCallUuid();
  void setCallUuid(const std::string &call_uuid);

  void waitForTokenTerminationNotification();
  virtual void interrupt();

  void token_termination_event_callback(const std::string &token_uuid,
                                        const std::string &node_uuid,
                                        const Types::ExitStatusCode &status);
  bool isStartGuardOff();

protected:
  virtual void start();
  virtual void resume();

  virtual Outcome activity();
  virtual void cleanup();

private:
  std::string call_uuid_;
  boost::signals2::connection connection_;

  Types::ExitStatusCode called_token_status_ =
      Types::ExitStatusCode::FALSE_ALARM;
};

} /* namespace trace */

#endif /* TRACE_MODEL_CALLACTIVITY_HPP_ */
