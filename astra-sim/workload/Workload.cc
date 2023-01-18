/******************************************************************************
This source code is licensed under the MIT license found in the
LICENSE file in the root directory of this source tree.
*******************************************************************************/

#include "astra-sim/workload/Workload.hh"

#include "astra-sim/json.hpp"
#include "astra-sim/system/IntData.hh"
#include "astra-sim/system/MemEventHandlerData.hh"
#include "astra-sim/system/RecvPacketEventHandlerData.hh"
#include "astra-sim/system/SendPacketEventHandlerData.hh"
#include "astra-sim/system/WorkloadLayerHandlerData.hh"
#include "extern/graph_frontend/chakra/eg_feeder/GraphFeeder.h"

#include <iostream>

using namespace std;
using namespace AstraSim;
using namespace Chakra;
using json = nlohmann::json;

Workload::Workload(
    Sys* sys,
    string eg_filename,
    string comm_group_filename) {
  this->graph_feeder =
    new GraphFeeder(eg_filename + "." + to_string(sys->id) + ".eg");
  this->comm_group = nullptr;
  // TODO: parametrize the number of available hardware resources
  this->hw_resource = new HardwareResource(1);
  this->sys = sys;
  initialize_comm_group(comm_group_filename);
  this->is_finished = false;
}

Workload::~Workload() {
  if (this->comm_group != nullptr)
    delete this->comm_group;
  if (this->graph_feeder != nullptr)
    delete this->graph_feeder;
}

void Workload::initialize_comm_group(string comm_group_filename)
{
  // communicator group input file is not given
  if (comm_group_filename.find("empty") != std::string::npos) {
    comm_group = nullptr;
    return;
  }

  ifstream inFile;
  json j;
  inFile.open(comm_group_filename);
  inFile >> j;

  for (json::iterator it = j.begin(); it != j.end(); ++it) {
    bool in_comm_group = false;

    for (auto id: it.value()) {
      if (id == sys->id) {
        in_comm_group = true;
      }
    }

    if (in_comm_group) {
      std::vector<int> involved_NPUs;
      for (auto id: it.value()) {
        involved_NPUs.push_back(id);
      }
      comm_group = new CommunicatorGroup(1, involved_NPUs, sys);
      // Note: All NPUs should create comm group with identical ids if they want to communicate with each other
    }
  }
}

void Workload::issue_dep_free_nodes() {
  std::queue<GraphNode*> push_back_queue;
  GraphNode* node = graph_feeder->getNextIssuableNode();
  while (node != nullptr) {
    if (hw_resource->is_available(node)) {
      issue(node);
    } else {
      push_back_queue.push(node);
    }
    node = graph_feeder->getNextIssuableNode();
  }

  while (!push_back_queue.empty()) {
    GraphNode* node = push_back_queue.front();
    graph_feeder->pushBackIssuableNode(node->id);
    push_back_queue.pop();
  }
}

void Workload::issue(GraphNode* node) {
  if (node->node_type == GraphNodeType::COMP_NODE) {
    if (node->simulated_run_time == 0) {
      skip_invalid(node);
    } else {
      if (sys->trace_enabled) {
        cout << "issue,sys->id=" << sys->id
          << ",tick=" << Sys::boostedTick()
          << ",node->id=" << node->id
          << ",node->name=" << node->name << endl;
      }
      issue_comp(node);
    }
  } else if ((node->node_type == GraphNodeType::COMM_COLL_NODE)
      || (node->node_type == GraphNodeType::COMM_SEND_NODE)
      || (node->node_type == GraphNodeType::COMM_RECV_NODE)) {
    if (sys->trace_enabled) {
      cout << "issue,sys->id=" << sys->id
        << ",tick=" << Sys::boostedTick()
        << ",node->id=" << node->id
        << ",node->name=" << node->name << endl;
    }
    issue_comm(node);
  } else if (node->node_type == GraphNodeType::INVALID_NODE) {
    skip_invalid(node);
  }
}

void Workload::issue_comp(GraphNode* node) {
  assert(node->node_type == GraphNodeType::COMP_NODE);
  hw_resource->occupy(node);

  WorkloadLayerHandlerData* wlhd = new WorkloadLayerHandlerData;
  wlhd->node_id = node->id;

  sys->register_event(
      this,
      EventType::General,
      wlhd,
      node->simulated_run_time);
}

void Workload::issue_comm(GraphNode* node) {
  int src, dst;

  hw_resource->occupy(node);

  if (node->node_type == GraphNodeType::COMM_COLL_NODE) {
    vector<bool> involved_dimensions = node->involved_dim;
    if (node->comm_type == CollectiveCommType::ALL_REDUCE) {
      DataSet *fp = sys->generate_all_reduce(
          node->comm_size,
          involved_dimensions,
          comm_group,
          node->comm_priority);
      collective_comm_node_id_map[fp->my_id] = node->id;
      fp->set_notifier(this, EventType::CollectiveCommunicationFinished);

    } else if (node->comm_type == CollectiveCommType::ALL_TO_ALL) {
      DataSet *fp = sys->generate_all_to_all(
          node->comm_size,
          involved_dimensions,
          comm_group,
          node->comm_priority);
      collective_comm_node_id_map[fp->my_id] = node->id;
      fp->set_notifier(this, EventType::CollectiveCommunicationFinished);

    } else if (node->comm_type == CollectiveCommType::ALL_GATHER) {
      DataSet *fp = sys->generate_all_gather(
          node->comm_size,
          involved_dimensions,
          comm_group,
          node->comm_priority);
      collective_comm_node_id_map[fp->my_id] = node->id;
      fp->set_notifier(this, EventType::CollectiveCommunicationFinished);

    } else if (node->comm_type == CollectiveCommType::REDUCE_SCATTER) {
      DataSet *fp = sys->generate_reduce_scatter(
          node->comm_size,
          involved_dimensions,
          comm_group,
          node->comm_priority);
      collective_comm_node_id_map[fp->my_id] = node->id;
      fp->set_notifier(this, EventType::CollectiveCommunicationFinished);

    }
  } else if (node->node_type == GraphNodeType::COMM_SEND_NODE) {
    sim_request snd_req;
    snd_req.srcRank = node->comm_src;
    snd_req.dstRank = node->comm_dst;
    snd_req.reqType = UINT8;
    SendPacketEventHandlerData *sehd = new SendPacketEventHandlerData;
    sehd->callable = this;
    sehd->wlhd = new WorkloadLayerHandlerData;
    sehd->wlhd->node_id = node->id;
    sehd->event = EventType::PacketSent;
    sys->front_end_sim_send(
            0,
            Sys::dummy_data,
            node->comm_size,
            UINT8,
            node->comm_dst,
            node->comm_tag,
            &snd_req,
            &Sys::handleEvent,
            sehd);
  } else if (node->node_type == GraphNodeType::COMM_RECV_NODE) {
    sim_request rcv_req;
    RecvPacketEventHandlerData *rcehd = new RecvPacketEventHandlerData;
    rcehd->wlhd = new WorkloadLayerHandlerData;
    rcehd->wlhd->node_id = node->id;
    rcehd->workload = this;
    rcehd->event = EventType::PacketReceived;
    sys->front_end_sim_recv(
            0,
            Sys::dummy_data,
            node->comm_size,
            UINT8,
            node->comm_src,
            node->comm_tag,
            &rcv_req,
            &Sys::handleEvent,
            rcehd);
  }
}

void Workload::skip_invalid(GraphNode* node) {
  graph_feeder->freeChildrenNodes(node->id);
  graph_feeder->removeNode(node->id);
}

void Workload::call(EventType event, CallData* data) {
  if (is_finished) {
    return;
  }

  if (event == EventType::CollectiveCommunicationFinished) {
    IntData* int_data = (IntData*)data;
    NodeId node_id = collective_comm_node_id_map[int_data->data];
    GraphNode* node = graph_feeder->lookupNode(node_id);

    if (sys->trace_enabled) {
      cout << "callback,sys->id=" << sys->id
        << ",tick=" << Sys::boostedTick()
        << ",node->id=" << node->id
        << ",node->name=" << node->name << endl;
    }

    hw_resource->release(node);

    graph_feeder->freeChildrenNodes(node_id);

    issue_dep_free_nodes();

    graph_feeder->removeNode(node_id);

  } else {
    if (data == nullptr) {
      issue_dep_free_nodes();
    } else {
      WorkloadLayerHandlerData *wlhd = (WorkloadLayerHandlerData *)data;
      GraphNode* node = graph_feeder->lookupNode(wlhd->node_id);

      if (sys->trace_enabled) {
        cout << "callback,sys->id=" << sys->id
          << ",tick=" << Sys::boostedTick()
          << ",node->id=" << node->id
          << ",node->name=" << node->name << endl;
      }

      hw_resource->release(node);

      graph_feeder->freeChildrenNodes(node->id);

      issue_dep_free_nodes();

      graph_feeder->removeNode(wlhd->node_id);
      delete wlhd;
    }
  }

  if (!graph_feeder->hasNodesToIssue()
      && (hw_resource->num_in_flight_comps == 0)
      && (hw_resource->num_in_flight_comms == 0)) {
    report();
    is_finished = true;
  }
}

void Workload::fire() {
  call(EventType::General, NULL);
}

void Workload::report() {
  Tick curr_tick = Sys::boostedTick();
  cout << "sys[" << sys->id << "] finished, " << curr_tick << " cycles" << endl;
}
