/******************************************************************************
This source code is licensed under the MIT license found in the
LICENSE file in the root directory of this source tree.
*******************************************************************************/

#ifndef __RENDEZVOUSRECVDATA_HH__
#define __RENDEZVOUSRECVDATA_HH__

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
#include "ns3/BasicEventHandlerData.hh"
#include "ns3/Common.hh"
#include "ns3/SimRecvCaller.hh"

namespace AstraSim {
class Sys;
class RendezvousRecvData : public BasicEventHandlerData, public MetaData {
 public:
  SimRecvCaller* recv;
  RendezvousRecvData(
      int nodeId,
      Sys* generator,
      void* buffer,
      uint64_t count,
      int type,
      int src,
      int tag,
      sim_request request,
      void (*msg_handler)(void* fun_arg),
      void* fun_arg);
};
} // namespace AstraSim
#endif
