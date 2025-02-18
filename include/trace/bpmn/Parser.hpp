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

// base
#include <trace/ModelEvent.hpp>
#include <trace/Object.hpp>
#include <trace/Types.hpp>

#include "trace/Outcome.hpp"

// std
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// boost xml parser
#include <boost/foreach.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// model
#include <trace/model/EscalationEndEvent.hpp>
#include <trace/model/MessageEndEvent.hpp>
#include <trace/model/SignalEndEvent.hpp>
#include <trace/model/TerminationEndEvent.hpp>

#include <trace/model/ExclusiveGateway.hpp>
#include <trace/model/InclusiveGateway.hpp>
#include <trace/model/ParallelGateway.hpp>

#include <trace/model/SequenceFlow.hpp>

#include <trace/model/MessageStartEvent.hpp>
#include <trace/model/SignalStartEvent.hpp>

#include <trace/model/CallActivity.hpp>

#include <trace/model/ScriptTask.hpp>
#include <trace/model/ServiceTask.hpp>
#include <trace/model/UserTask.hpp>

#include <trace/model/Process.hpp>

#include <trace/model/ErrorBoundaryEvent.hpp>
#include <trace/model/EscalationBoundaryEvent.hpp>
#include <trace/model/MessageBoundaryEvent.hpp>
#include <trace/model/SignalBoundaryEvent.hpp>
#include <trace/model/timer_boundary_event.h>

#include <trace/model/EscalationIntermediateThrowEvent.hpp>
#include <trace/model/MessageIntermediateThrowEvent.hpp>
#include <trace/model/SignalIntermediateThrowEvent.hpp>

#include <trace/model/EscalationIntermediateCatchEvent.hpp>
#include <trace/model/MessageIntermediateCatchEvent.hpp>
#include <trace/model/SignalIntermediateCatchEvent.hpp>
#include <trace/model/timer_intermediate_catch_event.h>

// model extension
#include <trace/model/extension/ConnectorServiceTask.hpp>
#include <trace/model/extension/ConnectorUserTask.hpp>

// Boost
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>

namespace trace {

class Parser {
public:
  Outcome extractModelFromXml(
      const std::string &url,
      std::unordered_map<std::string, std::shared_ptr<Process>> &processes,
      std::unordered_map<std::string, std::shared_ptr<ModelEvent>> &events);

private:
  static const std::string TAG;

  void extractModelEventsFromXml(
      const boost::property_tree::ptree &mission_model_parse_tree,
      std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
          &events_by_uuid_map);

  std::vector<std::shared_ptr<Process>> createProcessDefinition(
      const boost::property_tree::ptree::value_type &process,
      std::shared_ptr<Process> node,
      const std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
          &events);

  // task extraction
  // std::vector<std::shared_ptr<ScriptTask>> extractScriptTasks(const
  // boost::property_tree::ptree::value_type &root);
  std::vector<std::shared_ptr<ServiceTask>>
  extractServiceTasks(const boost::property_tree::ptree::value_type &root);

  std::vector<std::shared_ptr<UserTask>>
  extractUserTasks(const boost::property_tree::ptree::value_type &root);

  // activity extraction
  template <typename T>
  std::vector<std::shared_ptr<ProcessModelNode>> extractProcessModelNodesByType(
      const boost::property_tree::ptree::value_type &root,
      const std::string &type);

  std::vector<std::shared_ptr<CallActivity>>
  extractCallActivities(const boost::property_tree::ptree::value_type &root);

  // event extraction
  std::vector<std::shared_ptr<StartEvent>> extractStartEvents(
      const boost::property_tree::ptree::value_type &root,
      const std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
          &events);
  std::vector<std::shared_ptr<EndEvent>> extractEndEvents(
      const boost::property_tree::ptree::value_type &root,
      const std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
          &events);

  std::vector<std::shared_ptr<BoundaryEvent>> extractBoundaryEvents(
      const boost::property_tree::ptree::value_type &root,
      const std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
          &events);

  std::vector<std::shared_ptr<IntermediateCatchEvent>>
  extractIntermediateCatchEventsByType(
      const boost::property_tree::ptree::value_type &root,
      const std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
          &events);
  std::vector<std::shared_ptr<IntermediateThrowEvent>>
  extractIntermediateThrowEventsByType(
      const boost::property_tree::ptree::value_type &root,
      const std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
          &events);

  // subprocess extraction
  std::vector<std::shared_ptr<Process>> extractSubprocesses(
      const boost::property_tree::ptree::value_type &root,
      const std::unordered_map<std::string, std::shared_ptr<ModelEvent>>
          &events);

  // gateway extraction
  std::vector<std::shared_ptr<Gateway>>
  extractGateways(const boost::property_tree::ptree::value_type &root);

  // flow extraction
  template <typename T>
  std::vector<std::shared_ptr<Flow>>
  extractFlowsByType(const boost::property_tree::ptree::value_type &root,
                     const std::string &type);

  // data store utility functions
  void copyDataInputAssociationToLocalDataStore(
      const boost::property_tree::ptree::value_type &root,
      const boost::property_tree::ptree::value_type &node,
      std::shared_ptr<ProcessModelNode> &model_node);

  //
  void
  addPropertiesToNode(const boost::property_tree::ptree::value_type &bpmn_node,
                      ProcessModelNode *model_node);
};

} /* namespace trace */
