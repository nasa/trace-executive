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

#ifndef TRACE_MODEL_SEQUENCEFLOW_HPP_
#define TRACE_MODEL_SEQUENCEFLOW_HPP_

#include <trace/model/Flow.hpp>

namespace trace {

class SequenceFlow : public Flow {
public:
  SequenceFlow(const std::string &uuid, const std::string &name);
  SequenceFlow(const SequenceFlow &sequence_flow);
  virtual ~SequenceFlow(){};

  COPY_SUPPORT_FUNCTIONS(SequenceFlow);

  void addExpression(const std::string &expression);
  bool evaluateExpression();

  typedef struct Expression {
    std::string variable_type, rhs_variable, lhs_variable, operator_type;
  } Expression;

protected:
  template <class T>
  bool compare_arithmetic_using_operator(const T &lhs, const std::string &op,
                                         const T &rhs);

  bool compare_text_using_operator(const std::string &lhs,
                                   const std::string &op,
                                   const std::string &rhs);

private:
  Expression expression_;
};

} /* namespace trace */

#include <trace/model/SequenceFlow.tpp>

#endif /* TRACE_MODEL_SEQUENCEFLOW_HPP_ */
