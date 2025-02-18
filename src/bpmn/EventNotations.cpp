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

#include "trace/bpmn/EventNotations.hpp"

namespace trace {

EventNotations::EventNotations() {

  EventNotation signal_notation = {EventDefinition::SIGNAL, "signal",
                                   "bpmn:signal", "bpmn:signalEventDefinition",
                                   "<xmlattr>.signalRef"};
  event_notation_set_.push_back(signal_notation);

  EventNotation message_notation = {
      EventDefinition::MESSAGE, "message", "bpmn:message",
      "bpmn:messageEventDefinition", "<xmlattr>.messageRef"};
  event_notation_set_.push_back(message_notation);

  EventNotation escalation_notation = {
      EventDefinition::ESCALATION, "escalation", "bpmn:escalation",
      "bpmn:escalationEventDefinition", "<xmlattr>.escalationRef"};
  event_notation_set_.push_back(escalation_notation);

  EventNotation error_notation = {EventDefinition::ERROR, "error", "bpmn:error",
                                  "bpmn:errorEventDefinition",
                                  "<xmlattr>.errorRef"};
  event_notation_set_.push_back(error_notation);

  EventNotation termination_notation = {EventDefinition::TERMINATION,
                                        "termination", "",
                                        "bpmn:terminateEventDefinition", ""};
  event_notation_set_.push_back(termination_notation);
}

EventNotations::EventNotation EventNotations::getEventNotationByGlobalXmlAnchor(
    const std::string &global_xml_anchor) {
  const events_ordered_by_global_xml_anchor_index_t &index =
      event_notation_set_.get<event_global_xml_anchor_t>();
  events_ordered_by_global_xml_anchor_iterator_t global_xml_anchor_iterator =
      index.find(global_xml_anchor);
  if (global_xml_anchor_iterator != index.end()) {
    return (*global_xml_anchor_iterator);
  } else {
    throw std::out_of_range("Unable to find event notation by XML anchor [" +
                            global_xml_anchor + "] in set of event notations.");
  }
}

EventNotations::EventNotation
EventNotations::getEventNotationByDefinitionXmlAnchor(
    const std::string &definition_xml_anchor) {
  const events_ordered_by_definition_xml_anchor_index_t &index =
      event_notation_set_.get<event_definition_xml_anchor_t>();
  events_ordered_by_definition_xml_anchor_iterator_t
      definition_xml_anchor_iterator = index.find(definition_xml_anchor);
  if (definition_xml_anchor_iterator != index.end()) {
    return (*definition_xml_anchor_iterator);
  } else {
    throw std::out_of_range("Unable to find event notation by XML anchor [" +
                            definition_xml_anchor +
                            "] in set of event notations.");
  }
}

} // namespace trace
