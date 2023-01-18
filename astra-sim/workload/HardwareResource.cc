/******************************************************************************
This source code is licensed under the MIT license found in the
LICENSE file in the root directory of this source tree.
*******************************************************************************/

#include "astra-sim/workload/HardwareResource.hh"

using namespace std;
using namespace AstraSim;
using namespace Chakra;

HardwareResource::HardwareResource(uint32_t num_npus)
  : num_npus(num_npus),
  num_in_flight_comps(0),
  num_in_flight_comms(0) {
}

void HardwareResource::occupy(const GraphNode* node) {
  if (node->node_type == GraphNodeType::COMP_NODE) {
    ++num_in_flight_comps;
  } else if ((node->node_type == GraphNodeType::COMM_SEND_NODE)
      || (node->node_type == GraphNodeType::COMM_RECV_NODE)
      || (node->node_type == GraphNodeType::COMM_COLL_NODE)) {
    ++num_in_flight_comms;
  }
}

void HardwareResource::release(const GraphNode* node) {
  if (node->node_type == GraphNodeType::COMP_NODE) {
    --num_in_flight_comps;
  } else if ((node->node_type == GraphNodeType::COMM_SEND_NODE)
      || (node->node_type == GraphNodeType::COMM_RECV_NODE)
      || (node->node_type == GraphNodeType::COMM_COLL_NODE)) {
    --num_in_flight_comms;
  }
}

bool HardwareResource::is_available(const GraphNode* node) const {
  if ((node->node_type == GraphNodeType::COMP_NODE)
      && (num_in_flight_comps >= num_npus)) {
    return false;
  } else {
    return true;
  }
}
