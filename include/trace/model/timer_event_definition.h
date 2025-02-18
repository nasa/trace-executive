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

#ifndef TRACE_MODEL_TIMER_EVENT_DEFINITION_H_
#define TRACE_MODEL_TIMER_EVENT_DEFINITION_H_

#include <chrono>
#include <condition_variable>
#include <future>
#include <string>

namespace trace {
namespace model {

/**
 * @brief The TimerEventDefinition is used to specalize time-based events in
 * BPMN, such as intermediate and boundary timer events. These definitions can
 * either be a point in time, cyclic, or simply a duration.
 *
 */
class TimerEventDefinition {

public:
  /**
   * @brief Returns a TimerEventDefinition that represents a specific date in
   * time, like a deadline.
   *
   * @param iso8601_date_time Date and time formatted according to the ISO8601
   * standard.
   * @return TimerEventDefinition An object that encapsulate a deadline.
   */
  static TimerEventDefinition TimeDate(const std::string &iso8601_date_time);

  /**
   * @brief Returns a TimerEventDefinition that represents a cyclic timer event
   * that happens with some defined periodicity.
   *
   * @param iso8601_duration Duration formatted according to the
   * ISO8601 standard.
   * @return TimerEventDefinition An object that encapsulates a periodic event.
   */
  static TimerEventDefinition TimeCycle(const std::string &iso8601_duration);

  /**
   * @brief Returns a TimerEventDefinition that represents a timer event that
   * occurs after some duration from the current point in time.
   *
   * @param iso8601_date_time Duration formatted according to the ISO8601
   * standard.
   * @return TimerEventDefinition An object that encapsulates a timeout.
   */
  static TimerEventDefinition TimeDuration(const std::string &iso8601_duration);

  // TimerEventDefinition(const TimerEventDefinition &blueprint);

  // bool WaitOn(std::future<void> &timer_interrupt);
  bool WaitOn(std::condition_variable &timer_interrupt,
              std::unique_lock<std::mutex> &timer_lock);

  std::chrono::milliseconds PeekDuration() const;
  std::chrono::time_point<std::chrono::system_clock> PeekTimePoint() const;

private:
  /**
   * @brief This is the internal type of the TimerEventDefinition, which can be
   * a date, cycle, or duration. See BPMN 2.0 specification.
   *
   */
  enum class Type : uint8_t { kDate = 0, kCycle, kDuration };

  /**
   * @brief Convert from ISO 8601 duration format to an internal duration.
   *
   * @param iso8601_duration Druation formatted according to the ISO8601
   * standard.
   * @return std::chrono::seconds The duration in seconds; otherwise, throws an
   * exception.
   */
  static std::chrono::milliseconds
  FromIso8601Duration(const std::string &iso8601_duration);

  /**
   * @brief Convert from ISO 8601 date time format to an internal time point.
   *
   * @param iso8601_date_time Date and time formatted according to ISO8601
   * standard.
   * @return std::chrono::time_point The point in time representing the input
   * date and time; otherwise, throwns an exception.
   */
  static std::chrono::time_point<std::chrono::system_clock>
  FromIso8601DateTime(const std::string &iso8601_date_time);

  /**
   * @brief Construct a new TimerEventDefinition with a specified duration. Must
   * be either a cycle or duration type of event.
   *
   * @param type The internal type of this TimerEventDefinition.
   * @param duration_ms A duration expressed in seconds.
   */
  TimerEventDefinition(const Type &type,
                       const std::chrono::milliseconds &duration_ms);

  /**
   * @brief Construct a new Timer Event Definition object
   *
   * @param type The internal type of this TimerEventDefinition.
   * @param time_utc A point in time.
   */
  TimerEventDefinition(
      const Type &type,
      const std::chrono::time_point<std::chrono::system_clock> &time_utc);

  TimerEventDefinition(const Type &type, const std::string &iso8601_expression);

  /**
   * @brief A union that either stores a duration or point in time depending
   * on which type of TimerEventDefintion this object encapsulates.
   *
   */
  union TimerEvent {
    std::chrono::milliseconds duration_ms;
    std::chrono::time_point<std::chrono::system_clock> time_utc;
    TimerEvent() { duration_ms = std::chrono::milliseconds(0); }
  };

  TimerEvent timer_event_;
  Type timer_definition_type_;

  std::string iso8601_expression_;

  static bool IsAnExpression(const std::string &iso8601_string);

  // std::promise<void> timer_interrupt_;
};

} // namespace model
} // namespace trace

#endif