/******************************************************************************
This source code is licensed under the MIT license found in the
LICENSE file in the root directory of this source tree.
*******************************************************************************/

#ifndef __DOUBLEBINARYTREEALLREDUCE_HH__
#define __DOUBLEBINARYTREEALLREDUCE_HH__

#include <assert.h>
#include <math.h>
#include <stdlib.h>
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
#include "ns3/Algorithm.hh"
#include "ns3/CallData.hh"
#include "ns3/Common.hh"
#include "ns3/BinaryTree.hh"

namespace AstraSim {
class DoubleBinaryTreeAllReduce : public Algorithm {
 public:
  enum class State {
    Begin,
    WaitingForTwoChildData,
    WaitingForOneChildData,
    SendingDataToParent,
    WaitingDataFromParent,
    SendingDataToChilds,
    End
  };
  void run(EventType event, CallData* data);
  // void call(EventType event,CallData *data);
  // void exit();
  int parent;
  int left_child;
  int reductions;
  int right_child;
  // BinaryTree *tree;
  BinaryTree::Type type;
  State state;
  DoubleBinaryTreeAllReduce(
      int id,
      int layer_num,
      BinaryTree* tree,
      uint64_t data_size,
      bool boost_mode);
  // void init(BaseStream *stream);
};
} // namespace AstraSim
#endif
