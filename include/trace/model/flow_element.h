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

#ifndef FLOW_ELEMENT_H_
#define FLOW_ELEMENT_H_

#include <string>

namespace trace {
namespace model {

/**
 * @brief
 * This type serves as the base class to all other model types. All BPMN
 * elements must have a universally unique identifer, and optionally, a
 * name label in the diagram.
 **/
class FlowElement {

public:
public:
  /**
   * @brief
   * Constructs a flow element with the universally unique identifer
   * and an optional name (label) from the BPMN diagram.
   *
   * @param uuid The universally unique identifier assigned to this element
   * by the BPMN modeler.
   * @param name An optional name assigned to this element by the BPMN modeler;
   * defaults to the empty string.
   */
  explicit FlowElement(const std::string &uuid, const std::string &name = "");

  virtual ~FlowElement() = 0;

  /**
   * @brief
   * Returns the universally unique identifier of this flow element.
   *
   * @result The universally unique identifer of this flow element.
   */
  std::string uuid() const;

  /**
   * @brief
   * Returns the name of this flow element, empty string if unnamed.
   *
   * @result The name of this flow element or the empty string if this flow
   * element is unnamed (unlabeled in diagram).
   */
  std::string name() const;

private:
  std::string uuid_, name_;
};

} // namespace model
} // namespace trace

#endif // FLOW_ELEMENT_H_