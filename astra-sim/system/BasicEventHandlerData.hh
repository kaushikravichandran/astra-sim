/******************************************************************************
This source code is licensed under the MIT license found in the
LICENSE file in the root directory of this source tree.
*******************************************************************************/

#ifndef __BASICEVENTHANDLERDATA_HH__
#define __BASICEVENTHANDLERDATA_HH__

#include <assert.h>
#include <math.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <list>
#include <map>
#include <sstream>
#include <tuple>
#include <vector>
#include "ns3/CallData.hh"
#include "ns3/Common.hh"

namespace AstraSim {
class Sys;
class BasicEventHandlerData : public CallData {
 public:
  Sys *node;
  EventType event;
  BasicEventHandlerData(Sys* node, EventType event);
};
} // namespace AstraSim
#endif
