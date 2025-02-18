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

#include "trace/log/logger.hpp"

#include <cstring>
#include <sstream>

// support functions

/**
 * \def TRACE_SOURCE_CODE_LOCATION
 * Returns a location object storing the function, file, and line number
 * of a log statement in the source code.
 */
#define TRACE_SOURCE_CODE_LOCATION                                             \
  trace::log::Location(                                                        \
      __FUNCTION__,                                                            \
      (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__),        \
      __LINE__)

/**
 * \def TRACE_LOG_NAMED(severity, tag, content)
 * Invokes the global logger to log some content with a tag along with its
 * severity and location information.
 */
#define TRACE_LOG_NAMED(severity, tag, content)                                \
  do {                                                                         \
    std::stringstream content_stream;                                          \
    content_stream << content;                                                 \
                                                                               \
    trace::log::_global_logger.log(severity, TRACE_SOURCE_CODE_LOCATION, tag,  \
                                   content_stream.str());                      \
  } while (false)

// user functions

/**
 * \def TRACE_LOG_DEBUG(tag, content)
 * Logs some content with a tag at the DEBUG severity level
 */
#define TRACE_LOG_DEBUG(tag, content)                                          \
  TRACE_LOG_NAMED(trace::log::SeverityLevel::debug(), tag, content)

/**
 * \def TRACE_LOG_INFO(tag, content)
 * Logs some content with a tag at the INFO severity level
 */
#define TRACE_LOG_INFO(tag, content)                                           \
  TRACE_LOG_NAMED(trace::log::SeverityLevel::info(), tag, content)

/**
 * \def TRACE_LOG_WARN(tag, content)
 * Logs some content with a tag at the WARN severity level
 */
#define TRACE_LOG_WARN(tag, content)                                           \
  TRACE_LOG_NAMED(trace::log::SeverityLevel::warn(), tag, content)

/**
 * \def TRACE_LOG_ERROR(tag, content)
 * Logs some content with a tag at the ERROR severity level
 */
#define TRACE_LOG_ERROR(tag, content)                                          \
  TRACE_LOG_NAMED(trace::log::SeverityLevel::error(), tag, content)

/**
 * \def TRACE_LOG_FATAL(tag, content)
 * Logs some content with a tag at the FATAL severity level
 */
#define TRACE_LOG_FATAL(tag, content)                                          \
  TRACE_LOG_NAMED(trace::log::SeverityLevel::fatal(), tag, content)
