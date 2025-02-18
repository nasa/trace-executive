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

#include "trace/log/file_output_target.hpp"

#include <iostream>

namespace trace {
namespace log {

FileOutputTarget::FileOutputTarget(const std::string &output_file_url)
    : file_(output_file_url, std::ios::binary) {}

FileOutputTarget::~FileOutputTarget() { this->file_.close(); }

void FileOutputTarget::write(const Message &message) {
  this->file_ << message.timestamp().toString("%Y-%m-%dT%H:%M:%SZ") << ", "
              << message.content() << std::endl;
}

} // namespace log
} // namespace trace