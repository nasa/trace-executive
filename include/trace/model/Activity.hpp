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

#ifndef TRACE_MODEL_ACTIVITY_HPP_
#define TRACE_MODEL_ACTIVITY_HPP_

#include <trace/model/BoundaryEvent.hpp>
#include <trace/model/ProcessModelNode.hpp>

#include <atomic>

namespace trace {

class Activity : public ProcessModelNode {

public:
  Activity(const std::string &uuid, const std::string &name);
  Activity(const Activity &reference);
  virtual ~Activity();

  COPY_SUPPORT_FUNCTIONS(Activity);

  void addBoundaryEvent(const std::string &uuid,
                        std::shared_ptr<BoundaryEvent> &boundary_event);
  std::unordered_map<std::string, std::shared_ptr<BoundaryEvent>>
  listBoundaryEvents();

  virtual void interrupt();
  void boundary_event_callback(const std::shared_ptr<ModelEvent> &event);

protected:
  std::unordered_map<std::string, std::shared_ptr<BoundaryEvent>>
      boundary_events_;
  std::vector<boost::signals2::connection> boundary_event_connections_;

  virtual void start();
  virtual void resume();
  virtual void cleanup();

  std::atomic<bool> is_interrupted_;
  std::string interrupting_boundary_event_uuid_;

private:
  void activateBoundaryEvents();
};

} /* namespace trace */

#endif /* TRACE_MODEL_ACTIVITY_HPP_ */
