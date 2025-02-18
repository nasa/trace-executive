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

#include "trace/control/mission_control.h"

// Boost
#include <boost/program_options.hpp>

// C++
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <memory>

namespace {
std::shared_ptr<trace::control::MissionControl> _mission_control;
}

int main(int argc, char **argv) {

  std::string mission_file_url, base_connector;
  std::vector<std::string> connectors;

  boost::program_options::options_description desc(
      "All available TRACE options");
  desc.add_options()("help,h",
                     "Display helpful information about all options.")(
      "mission,m",
      boost::program_options::value<std::string>()->value_name("path"),
      "Mission model (BPMN) to automatically launch on start")(
      "connector,c",
      boost::program_options::value<std::string>()->value_name("name"),
      "Default connector for logging and heartbeats")(
      "connectors,C",
      boost::program_options::value<std::vector<std::string>>()->value_name(
          "names"),
      "List of additional supported connectors");

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, desc), vm);
  boost::program_options::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  if (vm.count("connector") > 0) {
    _mission_control = std::make_shared<trace::control::MissionControl>(
        vm["connector"].as<std::string>());
  } else {
    _mission_control = std::make_shared<trace::control::MissionControl>();
  }

  if (vm.count("mission") > 0) {

    bool ok = _mission_control->ResumeFromUrl("trace-snapshot.bin");
    if (!ok) {
      ok = _mission_control->start(vm["mission"].as<std::string>());
    }

    // CTRL-C
    std::signal(SIGINT, [](int signal) {
      if (signal == SIGINT) {
        std::cout << "Using Ctrl-C to terminate TRACE." << std::endl;
        _mission_control->SuspendToUrl("trace-snapshot.bin");
        std::cout << "Ctrl-C was used to terminate TRACE!" << std::endl;
      }
    });
    // CTRL-C

    if (ok) {
      _mission_control->wait();
      std::signal(SIGINT, SIG_DFL);
      _mission_control->abort();
    }
  } else {
    std::cout << "No mission file was specified!" << std::endl;
  }

  return 0;
}
