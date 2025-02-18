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

#include "trace/types/EventDefinition.hpp"

// Boost
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index_container.hpp>

namespace trace {

class EventNotations {

private:
  struct EventNotation {
    EventDefinition definition;
    std::string type_name;
    std::string global_xml_anchor;
    std::string definition_xml_anchor;
    std::string reference_xml_anchor;
  };

  struct event_global_xml_anchor_t {};
  struct event_definition_xml_anchor_t {};

  typedef boost::multi_index_container<
      EventNotation,
      boost::multi_index::indexed_by<
          boost::multi_index::random_access<>,
          boost::multi_index::hashed_unique<
              boost::multi_index::tag<event_global_xml_anchor_t>,
              BOOST_MULTI_INDEX_MEMBER(EventNotation, std::string,
                                       global_xml_anchor)>,
          boost::multi_index::hashed_unique<
              boost::multi_index::tag<event_definition_xml_anchor_t>,
              BOOST_MULTI_INDEX_MEMBER(EventNotation, std::string,
                                       definition_xml_anchor)>>>
      EventNotationSet;

  typedef boost::multi_index::index<EventNotationSet,
                                    event_global_xml_anchor_t>::type
      events_ordered_by_global_xml_anchor_index_t;
  typedef boost::multi_index::index<
      EventNotationSet, event_global_xml_anchor_t>::type::const_iterator
      events_ordered_by_global_xml_anchor_iterator_t;

  typedef boost::multi_index::index<EventNotationSet,
                                    event_definition_xml_anchor_t>::type
      events_ordered_by_definition_xml_anchor_index_t;
  typedef boost::multi_index::index<
      EventNotationSet, event_definition_xml_anchor_t>::type::const_iterator
      events_ordered_by_definition_xml_anchor_iterator_t;

  EventNotationSet event_notation_set_;

public:
  EventNotations();

  EventNotation
  getEventNotationByGlobalXmlAnchor(const std::string &global_xml_anchor);
  EventNotation getEventNotationByDefinitionXmlAnchor(
      const std::string &definition_xml_anchor);
};

} // namespace trace
