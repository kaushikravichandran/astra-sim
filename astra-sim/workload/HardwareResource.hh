/******************************************************************************
This source code is licensed under the MIT license found in the
LICENSE file in the root directory of this source tree.
*******************************************************************************/

#ifndef __HARDWARE_RESOURCE_HH__
#define __HARDWARE_RESOURCE_HH__

#include <cstdint>

#include "extern/graph_frontend/chakra/eg_def/GraphNode.h"

namespace AstraSim {

class HardwareResource
{
public:
  HardwareResource(uint32_t num_npus);
  void occupy(const Chakra::GraphNode* node);
  void release(const Chakra::GraphNode* node);
  bool is_available(const Chakra::GraphNode* node) const;

  const uint32_t num_npus;
  uint32_t num_in_flight_comps;
  uint32_t num_in_flight_comms;
};

}

#endif /* __HARDWARE_RESOURCE_HH__ */
