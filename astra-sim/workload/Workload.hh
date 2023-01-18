/******************************************************************************
This source code is licensed under the MIT license found in the
LICENSE file in the root directory of this source tree.
*******************************************************************************/

#ifndef __WORKLOAD_HH__
#define __WORKLOAD_HH__

#include <string>

#include "astra-sim/system/Callable.hh"
#include "astra-sim/system/CommunicatorGroup.hh"
#include "astra-sim/workload/HardwareResource.hh"
#include "extern/graph_frontend/chakra/eg_feeder/GraphFeeder.h"

namespace AstraSim {

class Sys;

class Workload : Callable {
 public:
  Workload(
      Sys* sys,
      std::string eg_filename,
      std::string comm_group_filename);
  ~Workload();

  // communicator groups
  void initialize_comm_group(std::string comm_group_filename);

  // event-based simulation
  void issue_dep_free_nodes();
  void issue(Chakra::GraphNode* node);
  void issue_comp(Chakra::GraphNode* node);
  void issue_comm(Chakra::GraphNode* node);
  void skip_invalid(Chakra::GraphNode* node);
  void call(EventType event, CallData* data);
  void fire();

  // stats
  void report();

  Chakra::GraphFeeder* graph_feeder;
  CommunicatorGroup* comm_group;
  HardwareResource* hw_resource;
  Sys* sys;
  std::map<int, uint64_t> collective_comm_node_id_map;
  bool is_finished;
};

}

#endif /* __WORKLOAD_HH__ */
