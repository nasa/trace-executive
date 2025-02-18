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

#include <trace/model/UserTask.hpp>

namespace trace {

UserTask::UserTask(const std::string &uuid, const std::string &name)
    : Task(uuid, name, Types::TaskType::USER) {}

UserTask::UserTask(const UserTask &user_task) : Task(user_task) {
  // no-op
}

UserTask::~UserTask() {}

void UserTask::publishUserNotification() {
  TRACE_LOG_WARN(
      uuid(), "This function should not be called, because I am the parent.");
}

void UserTask::waitForUserReply() {
  while (isActive()) {
    std::unique_lock<std::mutex> lock(user_notification_mutex_);
    auto status = user_notification_received_.wait_for(
        lock, std::chrono::milliseconds(100));
    lock.unlock();
    if (status == std::cv_status::no_timeout) {
      return;
    }
    publishUserNotification();
  }
  throw std::runtime_error(
      "Cancelled before any user notifications were received!");
}

void UserTask::start() { Task::start(); }

void UserTask::cleanup() { Task::cleanup(); }

} /* namespace trace */
