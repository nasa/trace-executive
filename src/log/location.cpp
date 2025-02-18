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

#include "trace/log/location.hpp"

namespace trace {
namespace log {

Location::Location(const std::string &function_name,
                   const std::string &file_name, const int line_number)
    : function_name_(function_name), file_name_(file_name),
      line_number_(line_number) {
  // no-op
}

std::string Location::function_name() const { return function_name_; }

std::string Location::file_name() const { return file_name_; }

int Location::line_number() const { return line_number_; }

} // namespace log
} // namespace trace