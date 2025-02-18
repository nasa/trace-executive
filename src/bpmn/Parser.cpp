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

#include "trace/bpmn/Parser.hpp"

#include "trace/bpmn/EventNotations.hpp"

#include <map>
#include <set>

namespace trace {

const std::string Parser::TAG = "Parser";

// Main Parse
Outcome Parser::extractModelFromXml(
    const std::string &mission_model_url,
    std::unordered_map<std::string, std::shared_ptr<Process>> &processes,
    std::unordered_map<std::string, std::shared_ptr<ModelEvent>> &events) {

  boost::property_tree::ptree mission_model_parse_tree;

  try {
    boost::property_tree::read_xml(mission_model_url, mission_model_parse_tree);
  } catch (const boost::property_tree::xml_parser::xml_parser_error &e) {
    TRACE_LOG_ERROR(TAG, e.what());
    return Outcome(StatusCode::ERROR, e.what());
  }

  try {
    extractModelEventsFromXml(mission_model_parse_tree, events);

    for (auto process_tree :
         mission_model_parse_tree.get_child("bpmn:definitions")) {
      if (process_tree.first != "bpmn:process") {
        TRACE_LOG_INFO(TAG,
                       "Skipping (" << process_tree.first << ") tag in XML.");
        continue;
      }

      std::string process_uuid =
                      process_tree.second.get<std::string>("<xmlattr>.id"),
                  process_name = process_tree.second.get<std::string>(
                      "<xmlattr>.name", "");
      auto node = std::make_shared<Process>(process_uuid, process_name);

      for (auto subprocess :
           createProcessDefinition(process_tree, node, events)) {
        TRACE_LOG_INFO(TAG,
                       "Adding (" << subprocess->uuid() << ") to process set.");
        processes[subprocess->uuid()] = subprocess;
      }

      TRACE_LOG_INFO(TAG, "Adding (" << process_uuid << ") to process set.");
      processes[process_uuid] = node;
    }

    return Outcome(StatusCode::OK, "Parsed mission model file successfully.");
  } catch (const std::runtime_error &e) {
    return Outcome(StatusCode::ERROR, e.what());
  }
}

// For extracting all defined events
void Parser::extractModelEventsFromXml(
    const boost::property_tree::ptree &mission_model_parse_tree,
    std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
        &events_by_uuid_map) {

  for (auto event_node :
       mission_model_parse_tree.get_child("bpmn:definitions")) {
    try {
      auto event_notation =
          EventNotations().getEventNotationByGlobalXmlAnchor(event_node.first);

      std::string uuid(event_node.second.get<std::string>("<xmlattr>.id")),
          name(event_node.second.get<std::string>("<xmlattr>.name", ""));
      events_by_uuid_map[uuid] =
          std::make_shared<ModelEvent>(uuid, name, event_notation.definition);
      TRACE_LOG_INFO(TAG, "Event \"" << name << "\" [" << uuid << "] ("
                                     << event_notation.type_name
                                     << ") resolved.");
    } catch (const std::out_of_range & /* e */) {
      TRACE_LOG_INFO(TAG,
                     "Ignoring XML element <"
                         << event_node.first
                         << ">, because it is not a BPMN event definition.");
    }
  }
}

std::vector<std::shared_ptr<Process>> Parser::createProcessDefinition(
    const boost::property_tree::ptree::value_type &process,
    std::shared_ptr<Process> node,
    const std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
        &events) {

  // Creating Script Tasks
  // std::vector<std::shared_ptr<ScriptTask>> script_tasks =
  // extractScriptTasks(process); for (auto task : script_tasks) {
  //   node->addNodeByUuid(task->uuid(), task);
  // }

  addPropertiesToNode(process, node.get());

  // for (const auto &process_child : process.second) {
  // }

  // Creating Service Tasks
  std::vector<std::shared_ptr<ServiceTask>> service_tasks =
      extractServiceTasks(process);
  for (auto task : service_tasks) {
    node->addNodeByUuid(task->uuid(), task);
  }

  // Creating User Tasks
  std::vector<std::shared_ptr<UserTask>> user_tasks = extractUserTasks(process);
  for (auto task : user_tasks) {
    node->addNodeByUuid(task->uuid(), task);
  }

  // Creating Exclusive Gateway
  for (auto gateway : extractGateways(process)) {
    node->addNodeByUuid(gateway->uuid(), gateway);
  }

  // Creating Start Events
  for (auto event : extractStartEvents(process, events)) {
    node->addNodeByUuid(event->uuid(), event);
  }

  // Creating End Events
  for (auto event : extractEndEvents(process, events)) {
    node->addNodeByUuid(event->uuid(), event);
  }

  // Creating Intermediate Throw Events
  for (auto event : extractIntermediateThrowEventsByType(process, events)) {
    node->addNodeByUuid(event->uuid(), event);
  }

  // Creating Intermediate Catch Events
  for (auto event : extractIntermediateCatchEventsByType(process, events)) {
    node->addNodeByUuid(event->uuid(), event);
  }

  // Creating Subprocesses
  std::vector<std::shared_ptr<Process>> subprocesses =
      extractSubprocesses(process, events);
  for (auto subprocess : subprocesses) {
    node->addNodeByUuid(subprocess->uuid(), subprocess);
  }

  // Creating Call Activities
  std::vector<std::shared_ptr<CallActivity>> call_activities =
      extractCallActivities(process);
  for (auto activity : call_activities) {
    node->addNodeByUuid(activity->uuid(), activity);
  }

  // Creating Message Boundary Events
  for (auto event : extractBoundaryEvents(process, events)) {
    node->addNodeByUuid(event->uuid(), event);

    for (auto task : service_tasks) {
      if (event->getAttachedToUuid().compare(task->uuid()) == 0) {
        task->addBoundaryEvent(event->uuid(), event);
        TRACE_LOG_INFO(TAG, "Boundary event [" << event->uuid()
                                               << "] has been added to Task ["
                                               << task->uuid() << "].");
      }
    }

    // for (auto task : script_tasks) {
    //   if (event->getAttachedToUuid().compare(task->uuid()) == 0) {
    //     task->addBoundaryEvent(event->uuid(), event);
    //     TRACE_LOG_INFO(TAG, "Boundary event [" << event->uuid() << "] has
    //     been added to Task [" << task->uuid() << "].");
    //   }
    // }

    for (auto task : user_tasks) {
      if (event->getAttachedToUuid().compare(task->uuid()) == 0) {
        task->addBoundaryEvent(event->uuid(), event);
        TRACE_LOG_INFO(TAG, "Boundary event [" << event->uuid()
                                               << "] has been added to Task ["
                                               << task->uuid() << "].");
      }
    }

    for (auto subprocess : subprocesses) {
      if (event->getAttachedToUuid().compare(subprocess->uuid()) == 0) {
        subprocess->addBoundaryEvent(event->uuid(), event);
        TRACE_LOG_INFO(TAG, "Boundary event ["
                                << event->uuid()
                                << "] has been added to (Sub)Process ["
                                << subprocess->uuid() << "].");
      }
    }

    for (auto call_activity : call_activities) {
      if (event->getAttachedToUuid().compare(call_activity->uuid()) == 0) {
        call_activity->addBoundaryEvent(event->uuid(), event);
        TRACE_LOG_INFO(TAG, "Boundary event ["
                                << event->uuid()
                                << "] has been added to (Sub)Process ["
                                << call_activity->uuid() << "].");
      }
    }
  }

  // Creating Sequence Flow from sources and targets
  for (auto flow_node :
       extractFlowsByType<SequenceFlow>(process, "bpmn:sequenceFlow")) {
    auto flow = std::static_pointer_cast<SequenceFlow>(flow_node);
    node->getNodeByUuid(flow->getSourceUuid())
        ->addOutputPort(flow->getTargetUuid(), flow);
    node->getNodeByUuid(flow->getTargetUuid())
        ->addInputPort(flow->getSourceUuid());
    TRACE_LOG_INFO(TAG, "Sequence flow from ["
                            << flow->getSourceUuid() << "] to ["
                            << flow->getTargetUuid() << "] has been created.");
  }

  TRACE_LOG_INFO(TAG, "Process has been created!");
  return subprocesses;
}

///

// For arbitrary Process Model Nodes
template <typename T>
std::vector<std::shared_ptr<ProcessModelNode>>
Parser::extractProcessModelNodesByType(
    const boost::property_tree::ptree::value_type &root,
    const std::string &type) {
  std::vector<std::shared_ptr<ProcessModelNode>> node_set;

  for (boost::property_tree::ptree::value_type bpmn2node : root.second) {
    if (!(bpmn2node.first.compare(type.c_str()) == 0)) {
      continue;
    }
    node_set.push_back(std::make_shared<T>(
        bpmn2node.second.get<std::string>("<xmlattr>.id"),
        bpmn2node.second.get<std::string>("<xmlattr>.name", "")));
    TRACE_LOG_INFO(TAG,
                   node_set.back()->name()
                       << " [" << node_set.back()->uuid()
                       << "] has been created and inserted into the process.");
  }
  return node_set;
}

std::vector<std::shared_ptr<Gateway>>
Parser::extractGateways(const boost::property_tree::ptree::value_type &root) {
  std::vector<std::shared_ptr<Gateway>> node_set;

  // std::set<std::string> gateway_set = { "bpmn:exclusiveGateway",
  // "bpmn:inclusiveGateway", "bpmn:parallelGateway" };

  for (boost::property_tree::ptree::value_type bpmn2node : root.second) {
    // if (gateway_set.find(bpmn2node.first) == gateway_set.end()) {
    //   continue;
    // } else {
    //   TRACE_LOG_INFO(TAG, "Found tag: " << bpmn2node.first);
    // }

    std::shared_ptr<Gateway> node;
    if (bpmn2node.first == "bpmn:exclusiveGateway") {
      node = std::make_shared<ExclusiveGateway>(
          bpmn2node.second.get<std::string>("<xmlattr>.id"),
          bpmn2node.second.get<std::string>("<xmlattr>.name", ""));
    } else if (bpmn2node.first == "bpmn:inclusiveGateway") {
      node = std::make_shared<InclusiveGateway>(
          bpmn2node.second.get<std::string>("<xmlattr>.id"),
          bpmn2node.second.get<std::string>("<xmlattr>.name", ""));
    } else if (bpmn2node.first == "bpmn:parallelGateway") {
      node = std::make_shared<ParallelGateway>(
          bpmn2node.second.get<std::string>("<xmlattr>.id"),
          bpmn2node.second.get<std::string>("<xmlattr>.name", ""));
    } else {
      continue;
    }

    node_set.push_back(node);
    std::string default_flow_uuid =
        bpmn2node.second.get<std::string>("<xmlattr>.default", "");

    if (default_flow_uuid.empty()) {
      TRACE_LOG_INFO(
          TAG, node_set.back()->name()
                   << " [" << node_set.back()->uuid()
                   << "] has been created and inserted into the process.");
    } else {
      node_set.back()->setDefaultOutputFlow(default_flow_uuid);
      TRACE_LOG_INFO(TAG, node_set.back()->name()
                              << " [" << node_set.back()->uuid()
                              << "] has been created with a default output "
                                 "flow and inserted into the process.");
    }
  }
  return node_set;
}

void Parser::copyDataInputAssociationToLocalDataStore(
    const boost::property_tree::ptree::value_type &root,
    const boost::property_tree::ptree::value_type &node,
    std::shared_ptr<ProcessModelNode> &model_node) {

  // Fix this-Clean it up (both sections)
  if (node.second.get_child_optional(
          "bpmn:extensionElements.camunda:properties")) {
    for (auto property :
         node.second.get_child("bpmn:extensionElements.camunda:properties")) {
      if (!(property.first.compare("camunda:property") == 0)) {
        continue;
      }
      auto name = property.second.get<std::string>("<xmlattr>.name");
      auto value = property.second.get<std::string>("<xmlattr>.value");
      TRACE_LOG_INFO(TAG, "(name, value) = (" << std::string(name) << ", "
                                              << std::string(value) << ")");
      model_node->addToDataInputStore(name, value);
    }
  }

  for (auto data_association :
       node.second) { // children("bpmn:dataInputAssociation"))
    if (!(data_association.first.compare("bpmn:dataInputAssociation") == 0)) {
      continue;
    }
    auto data_obj_ref_id =
        data_association.second.get<std::string>("bpmn:sourceRef");
    for (
        auto data_obj_ref :
        root.second) { // root.find_child_by_attribute("bpmn:dataObjectReference",
                       // "id", data_obj_ref_id);
      if (data_obj_ref.first.compare("bpmn:dataObjectReference") == 0 &&
          data_obj_ref.second.get<std::string>("<xmlattr>.id")
                  .compare(data_obj_ref_id) == 0) {
        for (
            auto property : data_obj_ref.second.get_child(
                "bpmn:extensionElements.camunda:"
                "properties")) { //.child("camunda:properties").children("camunda:property"))
          if (!(property.first.compare("camunda:property") == 0)) {
            continue;
          }
          auto name = property.second.get<std::string>("<xmlattr>.name");
          auto value = property.second.get<std::string>("<xmlattr>.value");
          TRACE_LOG_INFO(TAG, "(name, value) = (" << std::string(name) << ", "
                                                  << std::string(value) << ")");
          model_node->addToDataInputStore(name, value);
        }
      }
    }
  }
}

// For Service Tasks
std::vector<std::shared_ptr<ServiceTask>> Parser::extractServiceTasks(
    const boost::property_tree::ptree::value_type &root) {
  std::vector<std::shared_ptr<ServiceTask>> node_set;

  for (boost::property_tree::ptree::value_type node : root.second) {
    if (!(node.first.compare("bpmn:serviceTask") == 0)) {
      continue;
    }

    if (node.second.get_child_optional(
            "bpmn:extensionElements.camunda:connector")) {
      auto connector_def =
          node.second.get_child("bpmn:extensionElements.camunda:connector");
      std::string connector_id(
          connector_def.get<std::string>("camunda:connectorId"));
      TRACE_LOG_INFO(TAG, "connector_id = " << connector_id);
      std::shared_ptr<ProcessModelNode> model_node =
          std::make_shared<ConnectorServiceTask>(
              node.second.get<std::string>("<xmlattr>.id"),
              node.second.get<std::string>("<xmlattr>.name", ""), connector_id);
      for (
          auto property : connector_def.get_child(
              "camunda:inputOutput")) { //.child("camunda:properties").children("camunda:property"))
        if (!(property.first.compare("camunda:inputParameter") == 0)) {
          continue;
        }
        auto name = property.second.get<std::string>("<xmlattr>.name");
        auto value = property.second.get_value<std::string>();
        TRACE_LOG_INFO(TAG, "(name, value) = (" << std::string(name) << ", "
                                                << std::string(value) << ")");
        model_node->addToDataInputStore(name, value);
      }
      node_set.push_back(std::dynamic_pointer_cast<ServiceTask>(model_node));
      TRACE_LOG_INFO(
          TAG, "Service task "
                   << model_node->name() << " [" << model_node->uuid()
                   << "] has been created and inserted into the process.");
    } else {
      TRACE_LOG_WARN(TAG, "No connector defined. Skipping.");
      continue;
    }
  }
  return node_set;
}

// For User Tasks
std::vector<std::shared_ptr<UserTask>>
Parser::extractUserTasks(const boost::property_tree::ptree::value_type &root) {
  std::vector<std::shared_ptr<UserTask>> node_set;

  for (boost::property_tree::ptree::value_type node : root.second) {
    if (!(node.first.compare("bpmn:userTask") == 0)) {
      continue;
    }
    std::string connector_id(
        node.second.get<std::string>("<xmlattr>.camunda:assignee", ""));

    if (connector_id.empty()) {
      TRACE_LOG_WARN(
          TAG,
          "No camunda:assignee specified for this userTask. Erroring out.");
      throw std::runtime_error(
          "No camunda:assignee specified for this userTask.");
    }

    auto connector_def = node.second.get_child("bpmn:extensionElements");
    std::shared_ptr<ProcessModelNode> model_node =
        std::make_shared<ConnectorUserTask>(
            node.second.get<std::string>("<xmlattr>.id"),
            node.second.get<std::string>("<xmlattr>.name", ""), connector_id);
    for (
        auto property : connector_def.get_child(
            "camunda:inputOutput")) { //.child("camunda:properties").children("camunda:property"))
      if (!(property.first.compare("camunda:inputParameter") == 0)) {
        continue;
      }
      auto name = property.second.get<std::string>("<xmlattr>.name");
      auto value = property.second.get_value<std::string>();
      TRACE_LOG_INFO(TAG, "(name, value) = (" << std::string(name) << ", "
                                              << std::string(value) << ")");
      model_node->addToDataInputStore(name, value);
    }
    node_set.push_back(std::dynamic_pointer_cast<UserTask>(model_node));
    TRACE_LOG_INFO(TAG,
                   "Service task "
                       << model_node->name() << " [" << model_node->uuid()
                       << "] has been created and inserted into the process.");

    // std::shared_ptr<ProcessModelNode> model_node =
    // std::dynamic_pointer_cast<UserTask>(Registry::instance().newNodeByType(class_name,
    // node.second.get<std::string>("<xmlattr>.id"),
    // node.second.get<std::string>("<xmlattr>.name","")));
    // copyDataInputAssociationToLocalDataStore(root, node, model_node);
    node_set.push_back(std::dynamic_pointer_cast<UserTask>(model_node));

    TRACE_LOG_INFO(TAG,
                   "User task "
                       << model_node->name() << " [" << model_node->uuid()
                       << "] has been created and inserted into the process.");
  }
  return node_set;
}

// For Sequence Flows
template <typename T>
std::vector<std::shared_ptr<Flow>>
Parser::extractFlowsByType(const boost::property_tree::ptree::value_type &root,
                           const std::string &type) {
  std::vector<std::shared_ptr<Flow>> node_set;

  for (boost::property_tree::ptree::value_type bpmn2node : root.second) {
    if (!(bpmn2node.first.compare(type.c_str()) == 0)) {
      continue;
    }
    auto node = std::make_shared<T>(
        bpmn2node.second.get<std::string>("<xmlattr>.id"),
        bpmn2node.second.get<std::string>("<xmlattr>.name", ""));
    node->setSourceAndTargetUuids(
        bpmn2node.second.get<std::string>("<xmlattr>.sourceRef", ""),
        bpmn2node.second.get<std::string>("<xmlattr>.targetRef", ""));

    std::string condition_node =
        bpmn2node.second.get<std::string>("bpmn:conditionExpression", "");
    if (!condition_node.empty()) {
      node->addExpression(condition_node);
    }
    node_set.push_back(node);
  }
  return node_set;
}

// For Start Events
std::vector<std::shared_ptr<StartEvent>> Parser::extractStartEvents(
    const boost::property_tree::ptree::value_type &root,
    const std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
        &events) {
  std::vector<std::shared_ptr<StartEvent>> node_set;

  for (auto bpmn2node : root.second) {
    if (!(bpmn2node.first.compare("bpmn:startEvent") == 0)) {
      continue;
    }
    std::string start_event_name(
        bpmn2node.second.get<std::string>("<xmlattr>.name", ""));
    std::string start_event_id(
        bpmn2node.second.get<std::string>("<xmlattr>.id"));

    boost::property_tree::ptree event_def;
    std::string event_def_type;

    for (auto bpmn2node_child : bpmn2node.second) {
      if (bpmn2node_child.first.find("EventDefinition") != std::string::npos) {
        event_def_type = bpmn2node_child.first;
        event_def = bpmn2node_child.second;
        continue;
      }
    }

    if (event_def_type.empty()) { // normal start event
      auto node =
          std::make_shared<StartEvent>(start_event_id, start_event_name);
      node_set.push_back(node);
      TRACE_LOG_INFO(TAG, node->name() << " [" << node->uuid() << "].");
    } else if (event_def_type.compare("bpmn:messageEventDefinition") == 0) {
      auto node =
          std::make_shared<MessageStartEvent>(start_event_id, start_event_name);

      auto msg_def = event_def; //.get_child("bpmn:messageEventDefinition");
      if (!msg_def.get<std::string>("<xmlattr>.messageRef", "").empty()) {
        node->attachEventReference(
            events.at(msg_def.get<std::string>("<xmlattr>.messageRef")));
      } else {
        TRACE_LOG_WARN(TAG, "Message reference for start event ["
                                << start_event_id << "] " << start_event_name
                                << "is not yet implemented!");
      }
      node_set.push_back(node);
      TRACE_LOG_INFO(TAG, node->name()
                              << " [" << node->uuid() << "] with reference ["
                              << node->getEventUuid() << "].");

    } else if (event_def_type.compare("bpmn:signalEventDefinition") == 0) {
      auto node =
          std::make_shared<SignalStartEvent>(start_event_id, start_event_name);

      auto sig_def = event_def; // .get_child("bpmn:signalEventDefinition");
      if (!sig_def.get<std::string>("<xmlattr>.signalRef", "").empty()) {
        node->attachEventReference(
            events.at(sig_def.get<std::string>("<xmlattr>.signalRef")));
      } else {
        TRACE_LOG_WARN(TAG, "Signal reference for start event ["
                                << start_event_id << "] " << start_event_name
                                << "is not yet implemented!");
      }
      node_set.push_back(node);
      TRACE_LOG_INFO(TAG, node->name()
                              << " [" << node->uuid() << "] with reference ["
                              << node->getEventUuid() << "].");

      //} else { //not sure if we need this case
      //    throw std::runtime_error(event_def_type + " is not yet
      //    implemented!");
    }
  }
  return node_set;
}

// For End Events
std::vector<std::shared_ptr<EndEvent>> Parser::extractEndEvents(
    const boost::property_tree::ptree::value_type &root,
    const std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
        &events) {

  std::vector<std::shared_ptr<EndEvent>> node_set;

  for (auto bpmn2node : root.second) {
    if (!(bpmn2node.first.compare("bpmn:endEvent") == 0)) {
      continue;
    }
    std::string end_event_name(
        bpmn2node.second.get<std::string>("<xmlattr>.name", ""));
    std::string end_event_id(bpmn2node.second.get<std::string>("<xmlattr>.id"));

    boost::property_tree::ptree event_def_tree;
    std::string event_def_type;

    for (auto event_def : bpmn2node.second) {
      if (event_def.first.find("EventDefinition") != std::string::npos) {
        event_def_type = event_def.first;
        event_def_tree = event_def.second;
        continue;
      }
    }

    try {
      auto event_notation =
          EventNotations().getEventNotationByDefinitionXmlAnchor(
              event_def_type);

      std::string ref_uuid = event_def_tree.get<std::string>(
          event_notation.reference_xml_anchor, "");

      std::shared_ptr<EndEvent> node = nullptr;

      if (ref_uuid.empty()) {
        if (event_notation.definition == EventDefinition::TERMINATION) {
          node = std::make_shared<TerminationEndEvent>(end_event_id,
                                                       end_event_name);
          node_set.push_back(node);
          TRACE_LOG_INFO(TAG, node->name() << " [" << node->uuid()
                                           << "as a termination end event.");
        } else {
          TRACE_LOG_WARN(TAG, "Reference for " << event_notation.type_name
                                               << " end event [" << end_event_id
                                               << "] " << end_event_name
                                               << "is not defined!");
          throw std::runtime_error("Reference for " + event_notation.type_name +
                                   " end event [" + end_event_id + "] " +
                                   end_event_name + "is not defined!");
        }
      } else {

        switch (event_notation.definition) {
        case EventDefinition::SIGNAL:
          node = std::make_shared<SignalEndEvent>(end_event_id, end_event_name);
          break;
        case EventDefinition::MESSAGE:
          node =
              std::make_shared<MessageEndEvent>(end_event_id, end_event_name);
          break;
        case EventDefinition::ESCALATION:
          node = std::make_shared<EscalationEndEvent>(end_event_id,
                                                      end_event_name);
          break;
        default:
          throw std::runtime_error(
              "Event definition not supported by end event.");
        }

        node->attachEventReference(events.at(ref_uuid));
        node_set.push_back(node);
        TRACE_LOG_INFO(TAG, node->name()
                                << " [" << node->uuid() << "] with reference ["
                                << node->getEventUuid() << "]");
      }

    } catch (const std::out_of_range &e) {
      auto node = std::make_shared<EndEvent>(end_event_id, end_event_name);
      node_set.push_back(node);
      TRACE_LOG_WARN(TAG, e.what());
      TRACE_LOG_INFO(TAG, node->name() << " [" << node->uuid() << "]");
    }

    // if (event_def_type.empty()) { //normal end event
    //
    // } else if (event_def_type.compare("bpmn:messageEventDefinition") == 0) {
    //   auto node = std::make_shared<MessageEndEvent>(end_event_id,
    //   end_event_name);
    //
    //   auto msg_def = event_def_tree; //
    //   .get_child("bpmn:messageEventDefinition"); if
    //   (!msg_def.get<std::string>("<xmlattr>.messageRef","").empty()) {
    //     node->attachEventReference(events.at(msg_def.get<std::string>("<xmlattr>.messageRef")));
    //   } else {
    //     TRACE_LOG_WARN(TAG, "Message reference for end event [" <<
    //     end_event_id << "] " << end_event_name << "is not yet implemented!");
    //   }
    //   node_set.push_back(node);
    //   TRACE_LOG_INFO(TAG, node->name() << " [" << node->uuid() <<
    //   "] with reference [" << node->getEventUuid() << "].");
    //
    // } else if (event_def_type.compare("bpmn:signalEventDefinition") == 0) {
    //   auto node = std::make_shared<SignalEndEvent>(end_event_id,
    //   end_event_name);
    //
    //   auto sig_def = event_def_tree;
    //   //.get_child("bpmn:signalEventDefinition"); if
    //   (!sig_def.get<std::string>("<xmlattr>.signalRef","").empty()) {
    //     node->attachEventReference(events.at(sig_def.get<std::string>("<xmlattr>.signalRef")));
    //   } else {
    //     TRACE_LOG_WARN(TAG, "Signal reference for end event [" <<
    //     end_event_id << "] " << end_event_name << "is not yet implemented!");
    //   }
    //   node_set.push_back(node);
    //   TRACE_LOG_INFO(TAG, node->name() << " [" << node->uuid() <<
    //   "] with reference [" << node->getEventUuid() << "].");
    //
    // } else if (event_def_type.compare("bpmn:escalationEventDefinition") == 0)
    // {
    //   auto node = std::make_shared<EscalationEndEvent>(end_event_id,
    //   end_event_name);
    //
    //   auto esc_def = event_def_tree;
    //   //.get_child("bpmn:escalationEventDefinition"); if
    //   (!esc_def.get<std::string>("<xmlattr>.escalationRef","").empty()) {
    //     node->attachEventReference(events.at(esc_def.get<std::string>("<xmlattr>.escalationRef")));
    //   } else {
    //     TRACE_LOG_WARN(TAG, "Escalation reference for end event [" <<
    //     end_event_id << "] " << end_event_name << "is not yet implemented!");
    //   }
    //   node_set.push_back(node);
    //   TRACE_LOG_INFO(TAG, node->name() << " [" << node->uuid() <<
    //   "] with reference [" << node->getEventUuid() << "].");
    //
    // } else if (event_def_type.compare("bpmn:terminateEventDefinition") == 0)
    // {
    //   auto node = std::make_shared<TerminationEndEvent>(end_event_id,
    //   end_event_name); node_set.push_back(node); TRACE_LOG_INFO(TAG,
    //   node->name() << " [" << node->uuid() << "as a termination
    //   end event.");
    //
    // //} else { //not sure if we need this case
    // //    throw std::runtime_error(event_def_type + " is not yet
    // implemented!");
    // }
  }
  return node_set;
}

// For Boundary Events
std::vector<std::shared_ptr<BoundaryEvent>> Parser::extractBoundaryEvents(
    const boost::property_tree::ptree::value_type &root,
    const std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
        &events) {
  std::vector<std::shared_ptr<BoundaryEvent>> node_set;

  for (boost::property_tree::ptree::value_type bpmn2node :
       root.second) { // children("bpmn:boundaryEvent"))
    if (!(bpmn2node.first.compare("bpmn:boundaryEvent") == 0))
      continue;
    std::string boundary_event_id(
        bpmn2node.second.get<std::string>("<xmlattr>.id"));
    std::string boundary_event_name(
        bpmn2node.second.get<std::string>("<xmlattr>.name", ""));

    if (bpmn2node.second.get_child_optional("bpmn:messageEventDefinition")) {
      auto node = std::make_shared<MessageBoundaryEvent>(boundary_event_id,
                                                         boundary_event_name);

      auto event_def =
          bpmn2node.second.get_child("bpmn:messageEventDefinition");

      if (!event_def.get<std::string>("<xmlattr>.messageRef", "").empty()) {
        node->attachEventReference(
            events.at(event_def.get<std::string>("<xmlattr>.messageRef")));
      } else {
        TRACE_LOG_WARN(
            TAG,
            "Warning: There is no message event definition for boundary event! "
            "[" << boundary_event_id
                << "] " << boundary_event_name);
      }

      bool is_interrupting_event =
          bpmn2node.second.get<bool>("<xmlattr>.cancelActivity", true);
      node->attachedToUuid(
          bpmn2node.second.get<std::string>("<xmlattr>.attachedToRef"));
      node->cancelActivity(is_interrupting_event);

      TRACE_LOG_INFO(TAG, node->name()
                              << " [" << node->uuid() << "] attached to ["
                              << node->getAttachedToUuid()
                              << "] with cancelActivity:"
                              << node->getCancelActivity() << " and reference ["
                              << node->getEventUuid() << "].");

      addPropertiesToNode(bpmn2node, node.get());

      node_set.push_back(node);

    } else if (bpmn2node.second.get_child_optional(
                   "bpmn:signalEventDefinition")) {
      auto node = std::make_shared<SignalBoundaryEvent>(boundary_event_id,
                                                        boundary_event_name);

      auto event_def = bpmn2node.second.get_child("bpmn:signalEventDefinition");
      bool is_interrupting_event =
          bpmn2node.second.get<bool>("<xmlattr>.cancelActivity", true);
      node->attachedToUuid(
          bpmn2node.second.get<std::string>("<xmlattr>.attachedToRef"));
      node->cancelActivity(is_interrupting_event);

      if (!event_def.get<std::string>("<xmlattr>.signalRef", "").empty()) {
        node->attachEventReference(
            events.at(event_def.get<std::string>("<xmlattr>.signalRef")));

        TRACE_LOG_INFO(
            TAG, node->name()
                     << " [" << node->uuid() << "] attached to ["
                     << node->getAttachedToUuid()
                     << "] with cancelActivity:" << node->getCancelActivity()
                     << " and reference [" << node->getEventUuid() << "].");

      } else {
        TRACE_LOG_WARN(
            TAG,
            "Warning: There is no message event definition for boundary event! "
            "[" << boundary_event_id
                << "] " << boundary_event_name);
      }

      node_set.push_back(node);

    } else if (bpmn2node.second.get_child_optional(
                   "bpmn:escalationEventDefinition")) {
      auto node = std::make_shared<EscalationBoundaryEvent>(
          boundary_event_id, boundary_event_name);

      auto event_def =
          bpmn2node.second.get_child("bpmn:escalationEventDefinition");

      bool is_interrupting_event =
          bpmn2node.second.get<bool>("<xmlattr>.cancelActivity", true);
      node->attachedToUuid(
          bpmn2node.second.get<std::string>("<xmlattr>.attachedToRef"));
      node->cancelActivity(is_interrupting_event);

      if (!event_def.get<std::string>("<xmlattr>.escalationRef", "").empty()) {
        node->attachEventReference(
            events.at(event_def.get<std::string>("<xmlattr>.escalationRef")));
        TRACE_LOG_INFO(
            TAG, node->name()
                     << " [" << node->uuid() << "] attached to ["
                     << node->getAttachedToUuid()
                     << "] with cancelActivity:" << node->getCancelActivity()
                     << " and reference [" << node->getEventUuid() << "].");
      }

      node_set.push_back(node);
    } else if (bpmn2node.second.get_child_optional(
                   "bpmn:errorEventDefinition")) {
      auto node = std::make_shared<ErrorBoundaryEvent>(boundary_event_id,
                                                       boundary_event_name);

      auto event_def = bpmn2node.second.get_child("bpmn:errorEventDefinition");

      node->attachedToUuid(
          bpmn2node.second.get<std::string>("<xmlattr>.attachedToRef"));
      node->cancelActivity(true);

      if (!event_def.get<std::string>("<xmlattr>.errorRef", "").empty()) {
        node->attachEventReference(
            events.at(event_def.get<std::string>("<xmlattr>.errorRef")));
        TRACE_LOG_INFO(TAG, node->name()
                                << " [" << node->uuid() << "] attached to ["
                                << node->getAttachedToUuid()
                                << "] with reference [" << node->getEventUuid()
                                << "].");
      }

      node_set.push_back(node);
    } else if (bpmn2node.second.get_child_optional(
                   "bpmn:timerEventDefinition")) {

      auto event_definition =
          bpmn2node.second.get_child("bpmn:timerEventDefinition");

      std::array<std::string, 3> timer_definition_types = {
          "bpmn:timeDate", "bpmn:timeCycle", "bpmn:timeDuration"};

      size_t type_index = -1;
      std::string iso8601_formatted_string;

      for (size_t i = 0; i < timer_definition_types.size(); ++i) {
        auto timer_event_definition =
            event_definition.get_child_optional(timer_definition_types[i]);
        if (timer_event_definition) {
          iso8601_formatted_string =
              (*timer_event_definition).get_value<std::string>();
          type_index = i;
        }
      }

      std::shared_ptr<TimerBoundaryEvent> timer_boundary_event_node;

      switch (type_index) {
      case 0: // timeDate
        timer_boundary_event_node = std::make_shared<TimerBoundaryEvent>(
            boundary_event_id, boundary_event_name,
            model::TimerEventDefinition::TimeDate(iso8601_formatted_string));
        break;
      case 1: // timeCycle
        timer_boundary_event_node = std::make_shared<TimerBoundaryEvent>(
            boundary_event_id, boundary_event_name,
            model::TimerEventDefinition::TimeCycle(iso8601_formatted_string));
        break;
      case 2: // timeDuration
        timer_boundary_event_node = std::make_shared<TimerBoundaryEvent>(
            boundary_event_id, boundary_event_name,
            model::TimerEventDefinition::TimeDuration(
                iso8601_formatted_string));
        break;
      default:
        throw std::out_of_range(
            "No type is specified for this timer boundary event!");
      }

      bool is_interrupting_event =
          bpmn2node.second.get<bool>("<xmlattr>.cancelActivity", true);
      timer_boundary_event_node->attachedToUuid(
          bpmn2node.second.get<std::string>("<xmlattr>.attachedToRef"));
      timer_boundary_event_node->cancelActivity(is_interrupting_event);

      timer_boundary_event_node->attachEventReference(
          std::make_shared<ModelEvent>(boundary_event_id + "_timer",
                                       boundary_event_name,
                                       EventDefinition::TIMER));

      TRACE_LOG_INFO(TAG, timer_boundary_event_node->name()
                              << " [" << timer_boundary_event_node->uuid()
                              << "] attached to ["
                              << timer_boundary_event_node->getAttachedToUuid()
                              << "] with cancelActivity: " << std::boolalpha
                              << timer_boundary_event_node->getCancelActivity()
                              << ".");

      node_set.push_back(timer_boundary_event_node);
    } else {
      // TODO:  Add a "blank" boundary event -> this should not be accepted?
    }
  }
  return node_set;
} // namespace trace

// For Intermediate Throw Events
std::vector<std::shared_ptr<IntermediateThrowEvent>>
Parser::extractIntermediateThrowEventsByType(
    const boost::property_tree::ptree::value_type &root,
    const std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
        &events) {
  std::vector<std::shared_ptr<IntermediateThrowEvent>> node_set;

  for (boost::property_tree::ptree::value_type bpmn2node :
       root.second) { // children(type.c_str())) {
    if (!(bpmn2node.first.compare("bpmn:intermediateThrowEvent") == 0)) {
      continue;
    }
    std::string intermediate_event_id(
        bpmn2node.second.get<std::string>("<xmlattr>.id"));
    std::string intermediate_event_name(
        bpmn2node.second.get<std::string>("<xmlattr>.name", ""));

    if (bpmn2node.second.get_child_optional("bpmn:messageEventDefinition")) {
      auto node = std::make_shared<MessageIntermediateThrowEvent>(
          intermediate_event_id, intermediate_event_name);

      auto event_def =
          bpmn2node.second.get_child("bpmn:messageEventDefinition");
      if (!event_def.get<std::string>("<xmlattr>.messageRef", "").empty()) {
        node->attachEventReference(
            events.at(event_def.get<std::string>("<xmlattr>.messageRef")));
        TRACE_LOG_INFO(TAG, node->name()
                                << " [" << node->uuid() << "] with reference ["
                                << node->getEventUuid() << "].");
      }
      node_set.push_back(node);

    } else if (bpmn2node.second.get_child_optional(
                   "bpmn:signalEventDefinition")) {
      auto node = std::make_shared<SignalIntermediateThrowEvent>(
          intermediate_event_id, intermediate_event_name);

      auto event_def = bpmn2node.second.get_child("bpmn:signalEventDefinition");
      if (!event_def.get<std::string>("<xmlattr>.signalRef", "").empty()) {
        node->attachEventReference(
            events.at(event_def.get<std::string>("<xmlattr>.signalRef")));
        TRACE_LOG_INFO(TAG, node->name()
                                << " [" << node->uuid() << "] with reference ["
                                << node->getEventUuid() << "].");
      }
      node_set.push_back(node);

    } else if (bpmn2node.second.get_child_optional(
                   "bpmn:escalationEventDefinition")) {
      auto node = std::make_shared<EscalationIntermediateThrowEvent>(
          intermediate_event_id, intermediate_event_name);

      auto event_def =
          bpmn2node.second.get_child("bpmn:escalationEventDefinition");
      if (!event_def.get<std::string>("<xmlattr>.escalationRef", "").empty()) {
        node->attachEventReference(
            events.at(event_def.get<std::string>("<xmlattr>.escalationRef")));
        TRACE_LOG_INFO(TAG, node->name()
                                << " [" << node->uuid() << "] with reference ["
                                << node->getEventUuid() << "].");
      }
      node_set.push_back(node);

    } else {
      node_set.push_back(std::make_shared<IntermediateThrowEvent>(
          intermediate_event_id, intermediate_event_name,
          EventDefinition::NONE));
    }
  }
  return node_set;
}

// For Intermediate Catch Events
std::vector<std::shared_ptr<IntermediateCatchEvent>>
Parser::extractIntermediateCatchEventsByType(
    const boost::property_tree::ptree::value_type &root,
    const std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
        &events) {
  std::vector<std::shared_ptr<IntermediateCatchEvent>> node_set;

  for (boost::property_tree::ptree::value_type bpmn2node : root.second) {
    if (!(bpmn2node.first.compare("bpmn:intermediateCatchEvent") == 0)) {
      continue;
    }
    std::string intermediate_event_id(
        bpmn2node.second.get<std::string>("<xmlattr>.id"));
    std::string intermediate_event_name(
        bpmn2node.second.get<std::string>("<xmlattr>.name", ""));

    if (bpmn2node.second.get_child_optional("bpmn:messageEventDefinition")) {
      auto node = std::make_shared<MessageIntermediateCatchEvent>(
          intermediate_event_id, intermediate_event_name);

      auto event_def =
          bpmn2node.second.get_child("bpmn:messageEventDefinition");
      if (!event_def.get<std::string>("<xmlattr>.messageRef", "").empty()) {
        node->attachEventReference(
            events.at(event_def.get<std::string>("<xmlattr>.messageRef")));
        TRACE_LOG_INFO(TAG, node->name()
                                << " [" << node->uuid() << "] with reference ["
                                << node->getEventUuid() << "].");
      }

      addPropertiesToNode(bpmn2node, node.get());

      node_set.push_back(node);

    } else if (bpmn2node.second.get_child_optional(
                   "bpmn:signalEventDefinition")) {
      auto node = std::make_shared<SignalIntermediateCatchEvent>(
          intermediate_event_id, intermediate_event_name);

      auto event_def = bpmn2node.second.get_child("bpmn:signalEventDefinition");
      if (!event_def.get<std::string>("<xmlattr>.signalRef", "").empty()) {
        node->attachEventReference(
            events.at(event_def.get<std::string>("<xmlattr>.signalRef")));
        TRACE_LOG_INFO(TAG, node->name()
                                << " [" << node->uuid() << "] with reference ["
                                << node->getEventUuid() << "].");
      }
      node_set.push_back(node);

    } else if (bpmn2node.second.get_child_optional(
                   "bpmn:escalationEventDefinition")) {
      auto node = std::make_shared<EscalationIntermediateCatchEvent>(
          intermediate_event_id, intermediate_event_name);

      auto event_def =
          bpmn2node.second.get_child("bpmn:escalationEventDefinition");
      if (!event_def.get<std::string>("<xmlattr>.escalationRef", "").empty()) {
        node->attachEventReference(
            events.at(event_def.get<std::string>("<xmlattr>.escalationRef")));
        TRACE_LOG_INFO(TAG, node->name()
                                << " [" << node->uuid() << "] with reference ["
                                << node->getEventUuid() << "].");
      }
      node_set.push_back(node);

    } else if (bpmn2node.second.get_child_optional(
                   "bpmn:timerEventDefinition")) {

      auto event_def = bpmn2node.second.get_child("bpmn:timerEventDefinition");

      std::array<std::string, 3> timer_definition_types = {
          "bpmn:timeDate", "bpmn:timeCycle", "bpmn:timeDuration"};

      size_t type_index = -1;
      std::string iso8601_formatted_string;

      for (size_t i = 0; i < timer_definition_types.size(); ++i) {
        auto timer_event_definition =
            event_def.get_child_optional(timer_definition_types[i]);
        if (timer_event_definition) {
          iso8601_formatted_string =
              (*timer_event_definition).get_value<std::string>();
          type_index = i;
        }
      }

      std::shared_ptr<TimerIntermediateCatchEvent> node;

      switch (type_index) {
      case 0: // timeDate
        node = std::make_shared<TimerIntermediateCatchEvent>(
            intermediate_event_id, intermediate_event_name,
            model::TimerEventDefinition::TimeDate(iso8601_formatted_string));
        break;
      case 1: // timeCycle
        node = std::make_shared<TimerIntermediateCatchEvent>(
            intermediate_event_id, intermediate_event_name,
            model::TimerEventDefinition::TimeCycle(iso8601_formatted_string));
        break;
      case 2: // timeDuration
        node = std::make_shared<TimerIntermediateCatchEvent>(
            intermediate_event_id, intermediate_event_name,
            model::TimerEventDefinition::TimeDuration(
                iso8601_formatted_string));
        break;
      default:
        throw std::out_of_range(
            "No type is specified for this timer boundary event!");
      }

      node_set.push_back(node);

    } else {
      // TODO: Add a blank IntermediateCatchEvent case here?
    }
  }
  return node_set;
}

// Subprocesses
std::vector<std::shared_ptr<Process>> Parser::extractSubprocesses(
    const boost::property_tree::ptree::value_type &root,
    const std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
        &events) {
  std::vector<std::shared_ptr<Process>> node_set;

  for (boost::property_tree::ptree::value_type subprocess : root.second) {
    if (!(subprocess.first.compare("bpmn:subProcess") == 0)) {
      continue;
    }
    TRACE_LOG_INFO(TAG,
                   ">>> Creating process definition for "
                       << subprocess.second.get<std::string>("<xmlattr>.id"));
    node_set.push_back(std::make_shared<Process>(
        subprocess.second.get<std::string>("<xmlattr>.id"),
        subprocess.second.get<std::string>("<xmlattr>.name", "")));
    for (auto subprocess_node :
         createProcessDefinition(subprocess, node_set.back(), events)) {
      node_set.push_back(subprocess_node);
    }
    TRACE_LOG_INFO(
        TAG, "<<< " << node_set.back()->name() << " ["
                    << node_set.back()->uuid()
                    << "] has been created and inserted into the process.");
  }
  return node_set;
}

// Call Activities
std::vector<std::shared_ptr<CallActivity>> Parser::extractCallActivities(
    const boost::property_tree::ptree::value_type &root) {
  std::vector<std::shared_ptr<CallActivity>> node_set;

  for (boost::property_tree::ptree::value_type activity :
       root.second) { // children("bpmn:callActivity")) {
    if (!(activity.first.compare("bpmn:callActivity") == 0)) {
      continue;
    }
    auto node = std::make_shared<CallActivity>(
        activity.second.get<std::string>("<xmlattr>.id"),
        activity.second.get<std::string>("<xmlattr>.name", ""));
    node->setCallUuid(
        activity.second.get<std::string>("<xmlattr>.calledElement", ""));
    node_set.push_back(node);
    TRACE_LOG_INFO(TAG,
                   node_set.back()->name()
                       << " [" << node_set.back()->uuid()
                       << "] has been created and inserted into the process.");
  }
  return node_set;
}

void Parser::addPropertiesToNode(
    const boost::property_tree::ptree::value_type &bpmn_node,
    ProcessModelNode *model_node) {
  auto extension_elements =
      bpmn_node.second.get_child_optional("bpmn:extensionElements");
  if (extension_elements) {
    auto camunda_properties =
        (*extension_elements).get_child_optional("camunda:properties");
    if (camunda_properties) {
      for (const auto &property : (*camunda_properties)) {

        std::string name = property.second.get<std::string>("<xmlattr>.name"),
                    value =
                        property.second.get<std::string>("<xmlattr>.value", "");

        TRACE_LOG_INFO(TAG, "Adding property (" << name << ", " << value
                                                << ") to model node ("
                                                << model_node->uuid() << ").");

        model_node->addToProperties(name, value);
      }
    }
  }
}

} /* namespace trace */
