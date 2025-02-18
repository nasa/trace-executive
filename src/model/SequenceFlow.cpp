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

#include <trace/model/SequenceFlow.hpp>
#include <trace/storage/DataStore.hpp>

#include <boost/regex.hpp>

#include <regex>
#include <type_traits>

namespace trace {

SequenceFlow::SequenceFlow(const std::string &uuid, const std::string &name)
    : Flow(uuid, name) {
  expression_.operator_type = "none";
}

SequenceFlow::SequenceFlow(const SequenceFlow &sequence_flow)
    : Flow(sequence_flow) {
  expression_ = sequence_flow.expression_;
}

void SequenceFlow::addExpression(const std::string &expression) {
  boost::regex re(":|==|!=|>=|>|<=|<");
  const int subs[] = {-1, 0};
  boost::sregex_token_iterator i(expression.begin(), expression.end(), re,
                                 subs),
      j;
  unsigned count = 0;
  std::vector<std::string> tokens;
  while (i != j) {
    tokens.push_back(*i++);
    count++;
  }
  if (count == 5) {
    expression_.variable_type = tokens[0];
    expression_.lhs_variable = tokens[2];
    expression_.operator_type = tokens[3];
    expression_.rhs_variable = tokens[4];
  } else {
    throw std::runtime_error(
        "Unable to parse expression, should be: <type>:lhs<operator>rhs");
  }
}

bool SequenceFlow::evaluateExpression() {

  if (expression_.operator_type.compare("none") != 0) {

    std::regex txt_regex("\\$\\{.*\\}");
    std::smatch txt_match;
    std::string rhs_value;

    if (std::regex_match(expression_.rhs_variable, txt_match, txt_regex)) {
      std::string s = expression_.rhs_variable.substr(
          2, expression_.rhs_variable.size() - 3);

      auto sub_index = s.find_first_of(",");
      std::string key = s.substr(0, sub_index),
                  default_value = s.substr(sub_index + 1);

      TRACE_LOG_INFO(uuid(), "Searching data store for key (" << key << ").");
      try {
        rhs_value = DataStore::instance().get_value_by_key<std::string>(key);
        TRACE_LOG_INFO(uuid(),
                       "Resolved (" << key << ", " << rhs_value << ").");
      } catch (const std::runtime_error &e) {
        if (default_value.empty()) {
          TRACE_LOG_WARN(uuid(), "Unable to resolve (" << key << ").");
        } else {
          rhs_value = default_value;
          TRACE_LOG_INFO(uuid(), "Resolved using default value ("
                                     << key << ", " << default_value << ").");
        }
      }
    } else {
      rhs_value = expression_.rhs_variable;
    }

    if (expression_.variable_type.compare("int") == 0) {
      int value = std::stoi(DataStore::instance().get_value_by_key<std::string>(
          expression_.lhs_variable));
      return compare_arithmetic_using_operator<int>(
          value, expression_.operator_type, std::stoi(rhs_value));
    } else if (expression_.variable_type.compare("double") == 0) {
      double value =
          std::stod(DataStore::instance().get_value_by_key<std::string>(
              expression_.lhs_variable));
      return compare_arithmetic_using_operator<double>(
          value, expression_.operator_type, std::stod(rhs_value));
    } else if (expression_.variable_type.compare("string") == 0) {
      std::string value = DataStore::instance().get_value_by_key<std::string>(
          expression_.lhs_variable);
      return compare_text_using_operator(value, expression_.operator_type,
                                         rhs_value);
    } else if (expression_.variable_type == "data") {
    }
  }
  return true; // no condition, therefore true
}

bool SequenceFlow::compare_text_using_operator(const std::string &lhs,
                                               const std::string &op,
                                               const std::string &rhs) {
  if (op.compare("==") == 0) {
    return (lhs.compare(rhs) == 0);
  } else if (op.compare("!=") == 0) {
    return (lhs.compare(rhs) != 0);
  } else if (op.compare(">") == 0) {
    return (lhs.compare(rhs) > 0);
  } else if (op.compare(">=") == 0) {
    return (lhs.compare(rhs) >= 0);
  } else if (op.compare("<") == 0) {
    return (lhs.compare(rhs) < 0);
  } else if (op.compare("<=") == 0) {
    return (lhs.compare(rhs) <= 0);
  }
  throw std::runtime_error("Unsupported operator detected. Unable to proceed.");
}

} /* namespace trace */
