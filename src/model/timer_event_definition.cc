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

#include "trace/model/timer_event_definition.h"
#include "trace/connector/ConnectorManager.hpp"
#include "trace/log/macros.hpp"
#include "trace/storage/DataStore.hpp"

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

#include <array>
#include <iostream>
#include <regex>

namespace trace {
namespace model {

TimerEventDefinition
TimerEventDefinition::TimeDate(const std::string &iso8601_date_time) {
  if (IsAnExpression(iso8601_date_time)) {
    return TimerEventDefinition(Type::kDate, iso8601_date_time);
  }
  return TimerEventDefinition(Type::kDate,
                              FromIso8601DateTime(iso8601_date_time));
}

TimerEventDefinition
TimerEventDefinition::TimeCycle(const std::string &iso8601_duration) {

  if (IsAnExpression(iso8601_duration)) {
    return TimerEventDefinition(Type::kCycle, iso8601_duration);
  } else {
    return TimerEventDefinition(Type::kCycle,
                                FromIso8601Duration(iso8601_duration));
  }
}

TimerEventDefinition
TimerEventDefinition::TimeDuration(const std::string &iso8601_duration) {
  if (IsAnExpression(iso8601_duration)) {
    return TimerEventDefinition(Type::kDuration, iso8601_duration);
  } else {
    return TimerEventDefinition(Type::kDuration,
                                FromIso8601Duration(iso8601_duration));
  }
}

std::chrono::system_clock::time_point TimerEventDefinition::FromIso8601DateTime(
    const std::string &iso8601_date_time) {
  auto date_time = boost::posix_time::from_iso_string(iso8601_date_time);
  if (date_time == boost::posix_time::ptime()) {
    throw std::invalid_argument(
        "Input provided is not a valid date and time in ISO8601.");
  }
  // }

  boost::posix_time::time_duration const time_since_epoch =
      date_time - boost::posix_time::from_time_t(0);

  std::chrono::time_point<std::chrono::system_clock> t =
      std::chrono::system_clock::from_time_t(time_since_epoch.total_seconds());

  long nsec = time_since_epoch.fractional_seconds() *
              (1000000000 / time_since_epoch.ticks_per_second());

  return (t + std::chrono::nanoseconds(nsec));
}

std::chrono::milliseconds
TimerEventDefinition::FromIso8601Duration(const std::string &iso8601_duration) {

  TRACE_LOG_INFO("timer_event_definition",
                  "FromIso8601Duration(" << iso8601_duration << ")");

  std::array<std::string, 2> iso8601_regex = {
      "P([[:d:]]+Y)?([[:d:]]+M)?([[:d:]]+D)?T([[:d:]]+H)?([[:d:]]+M)?"
          "([[:d:]]+S|[[:d:]]+\\.[[:d:]]+S)?",
      "P([[:d:]]+Y)?([[:d:]]+M)?([[:d:]]+D)?",
  };

  for (const auto &format : iso8601_regex) {

    boost::smatch match;
    boost::regex re(format);
    boost::regex_match(iso8601_duration, match, re);

    if (match.empty() == false) {

      std::array<double, 6> lut = {
          0, 0, 0, 0, 0, 0}; // years, months, days, hours, minutes, seconds

      TRACE_LOG_INFO("timer_event_definition", "format: " << format);

      bool matched = false;
      for (size_t i = 1; i < match.size(); ++i) {
        if (match[i].matched) {
          matched = true;
          std::string value = match[i];
          TRACE_LOG_INFO("timer_event_definition", value);
          value.pop_back(); // remove trailing character
          lut[i - 1] = boost::lexical_cast<double>(value);
        }
      }

      TRACE_LOG_INFO("timer_event_definition",
                      "(" << lut[0] << ", " << lut[1] << ", " << lut[2] << ", "
                          << lut[3] << ", " << lut[4] << ", " << lut[5] << ")");

      std::chrono::milliseconds duration_ms(
          static_cast<int64_t>((31556926 * lut[0] + 2629743.83 * lut[1] +
                               86400 * lut[2] + 3600 * lut[3] + 60 * lut[4] +
                               lut[5]) *
          1000));

      if (matched) {
          return duration_ms;
      }
    }
  }

  throw std::invalid_argument(
      "Input is not a properly formatted ISO8601 duration.");
}

// bool TimerEventDefinition::WaitOn(std::future<void> &timer_interrupt) {

//   TRACE_LOG_INFO("timer_event_definition",
//                   "About to wait on a future timer interrupt.");

//   std::future_status status = std::future_status::deferred;

//   switch (this->timer_definition_type_) {
//   case Type::kDate:
//     TRACE_LOG_INFO("timer_event_definition", "Hi, I'm a date!");
//     if (this->iso8601_expression_.empty() == false) {
//       this->timer_event_.time_utc = FromIso8601DateTime(
//           DataStore::instance().FromExpression(this->iso8601_expression_));
//     }
//     status = timer_interrupt.wait_until(this->timer_event_.time_utc);
//     break;
//   case Type::kCycle:
//   case Type::kDuration:
//     TRACE_LOG_INFO("timer_event_definition", "Hi, I'm a duration (or
//     cycle)!"); if (this->iso8601_expression_.empty() == false) {
//       auto iso8601_duration =
//           DataStore::instance().FromExpression(this->iso8601_expression_);
//       TRACE_LOG_INFO("timer_event_definition",
//                       "(iso8601_expression, iso8601_duration) = ("
//                           << iso8601_expression_ << ", " << iso8601_duration
//                           << ")");
//       this->timer_event_.duration_ms = FromIso8601Duration(iso8601_duration);
//     }
//     TRACE_LOG_INFO("timer_event_definition",
//                     "duration (ms): (" <<
//                     this->timer_event_.duration_ms.count()
//                                        << ")");
//     status = timer_interrupt.wait_for(this->timer_event_.duration_ms);
//     break;
//   }

//   return (status == std::future_status::timeout);
// }

bool TimerEventDefinition::WaitOn(std::condition_variable &timer_interrupt,
                                  std::unique_lock<std::mutex> &timer_lock) {

  bool is_timeout = false;
  auto connector = ConnectorManager::instance().default_connector();

  // std::cv_status status = std::cv_status::no_timeout;

  switch (this->timer_definition_type_) {
  case Type::kDate:
    if (this->iso8601_expression_.empty() == false) {
      this->timer_event_.time_utc = FromIso8601DateTime(
          DataStore::instance().FromExpression(this->iso8601_expression_));
    }

    if (connector == nullptr) {
      is_timeout = (timer_interrupt.wait_until(timer_lock,
                                               this->timer_event_.time_utc) ==
                    std::cv_status::timeout);
    } else {
      is_timeout = connector->wait_until(timer_interrupt, timer_lock,
                                         this->timer_event_.time_utc);
    }
    break;
  case Type::kCycle:
  case Type::kDuration:
    if (this->iso8601_expression_.empty() == false) {
      this->timer_event_.duration_ms = FromIso8601Duration(
          DataStore::instance().FromExpression(this->iso8601_expression_));
    }
    if (connector == nullptr) {
      is_timeout = (timer_interrupt.wait_for(timer_lock,
                                             this->timer_event_.duration_ms) ==
                    std::cv_status::timeout);
    } else {
      is_timeout = connector->wait_for(timer_interrupt, timer_lock,
                                       this->timer_event_.duration_ms);
    }
    break;
  }

  // return (status == std::cv_status::timeout);
  return is_timeout;
}

// void TimerEventDefinition::Cancel() {
//   try {
//     std::cout << "Trying to cancel timer..." << std::endl;
//     this->timer_interrupt_.set_value();
//     std::cout << "...cancelled." << std::endl;
//   } catch (...) {
//     std::cout << "Timer has already expired." << std::endl;
//   }
// }

TimerEventDefinition::TimerEventDefinition(
    const Type &type, const std::chrono::milliseconds &duration_ms)
    : timer_definition_type_(type) {
  timer_event_.duration_ms = duration_ms;
}

TimerEventDefinition::TimerEventDefinition(
    const Type &type, const std::string &iso8601_expression)
    : timer_definition_type_(type), iso8601_expression_(iso8601_expression) {}

TimerEventDefinition::TimerEventDefinition(
    const Type &type,
    const std::chrono::time_point<std::chrono::system_clock> &time_utc)
    : timer_definition_type_(type) {
  timer_event_.time_utc = time_utc;
}

std::chrono::milliseconds TimerEventDefinition::PeekDuration() const {
  if (this->timer_definition_type_ != Type::kDate) {
    return this->timer_event_.duration_ms;
  }
  return std::chrono::milliseconds::max();
}

std::chrono::time_point<std::chrono::system_clock>
TimerEventDefinition::PeekTimePoint() const {
  if (this->timer_definition_type_ == Type::kDate) {
    return this->timer_event_.time_utc;
  }
  return std::chrono::system_clock::time_point::max();
}

bool TimerEventDefinition::IsAnExpression(const std::string &iso8601_string) {
  const std::regex txt_regex("(\\$\\{.*?\\})");
  std::smatch txt_match;
  return std::regex_search(iso8601_string.cbegin(), iso8601_string.cend(),
                           txt_match, txt_regex);
}

} // namespace model
} // namespace trace
