#include "node.h"
#include <iostream>
#include <queue>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

using namespace std;

void RoutingNode::DJ_Algo(){
  unordered_map<string, int> djikstra_Dist_Tracker;
  unordered_map<string, string> nexthop_tracker;
  unordered_map<string, string> interface_nodeip_tracker;

  priority_queue<pair<int, string>, vector<pair<int, string>>, greater<pair<int, string>>> pq;

  for (int i = 0; i < neighbor_tbl.tbl.size(); i++){
    RoutingEntry entry = neighbor_tbl.tbl[i];
    djikstra_Dist_Tracker[entry.dstip] = entry.cost;
    nexthop_tracker[entry.dstip] = entry.nexthop;
    interface_nodeip_tracker[entry.dstip] = entry.ip_interface;
  }

  for (pair<NetInterface, Node*>& interface: interfaces){
    string src = interface.first.getip();
    string dest = interface.first.getConnectedIp();
    int cost = interface.first.getCost();
    djikstra_Dist_Tracker[dest] = cost;
    nexthop_tracker[dest] = dest;
    interface_nodeip_tracker[dest] = src;
    pq.push({cost, dest});
  }

  while (!pq.empty()){
    int best_distance_from_pq = pq.top().first;
    string currNode = pq.top().second;
    pq.pop();

    if (djikstra_Dist_Tracker[currNode] < best_distance_from_pq) continue;

    if (adjList.count(currNode) == 0) continue;

    for (Edge& edge: adjList[currNode]){
      string dest = edge.dst;
      int cost = edge.cost;
      if ((djikstra_Dist_Tracker.count(edge.dst) == 0) || (djikstra_Dist_Tracker[dest] > djikstra_Dist_Tracker[currNode] + cost)){
        djikstra_Dist_Tracker[dest] = djikstra_Dist_Tracker[currNode] + cost;
        nexthop_tracker[dest] = nexthop_tracker[currNode];
        interface_nodeip_tracker[dest] = interface_nodeip_tracker[currNode];
        pq.push({djikstra_Dist_Tracker[dest], dest});
      }
    }

    mytbl.tbl.clear();

    for (pair<string, int> p: djikstra_Dist_Tracker){
      RoutingEntry entry;
      entry.dstip = p.first;
      entry.nexthop = nexthop_tracker[p.first];
      entry.cost = p.second;
      entry.ip_interface = interface_nodeip_tracker[p.first];
      mytbl.tbl.push_back(entry);
    }
  }
}

void printRT(vector<RoutingNode*> nd){
/*Print routing table entries*/
	for (int i = 0; i < nd.size(); i++) {
	  nd[i]->printTable();
	}
}


void routingAlgo(vector<RoutingNode*> nd){
 
  bool saturation=false;
 
  for(int i=1; i<nd.size(); i++) {
    for (RoutingNode* node: nd){
      node->sendMsg();
    }
  }

  for (int i = 0; i < nd.size(); i++){
    RoutingNode* node = nd[i];
    node->DJ_Algo();
  }
  
  /*Print routing table entries after routing algo converges */
  printf("Printing the routing tables after the convergence \n");
  printRT(nd);
}

void RoutingNode::recvMsg(RouteMsg *msg) {
  //your code here
 
  // Traverse the routing table in the message.
  // Check if entries present in the message table is closer than already present 
  // entries.
  // Update entries.
 
  routingtbl *recvRoutingTable = msg->mytbl;
  
  // Build Graph from the received routing table

  vector<Edge> Nedges;
  vector<string> interface_ips_of_source;
  for (RoutingEntry r_entry: recvRoutingTable->tbl){
    if (r_entry.dstip == r_entry.ip_interface){
      interface_ips_of_source.push_back(r_entry.ip_interface);
    }
    else {
      Edge edge(r_entry);
      Nedges.push_back(edge);
    }
  }

  // Connect source interfaces to source IP 0 cost
  for (string& interface_ip: interface_ips_of_source){
    if (interface_ip == msg->from) continue;
    adjList.erase(interface_ip);
    adjList[interface_ip] = {Edge(msg->from, 0)};
    Nedges.push_back(Edge(interface_ip, 0));
  } 

  bool any_update = false;
  if (adjList.count(msg->from) == 0){
    adjList[msg->from] = Nedges;
    this->sendNbrData(msg->from, msg);
    return;
  }
  else{
    vector<Edge>& src_edges = adjList[msg->from];

    // Sort the edges based on dst
    sort(src_edges.begin(), src_edges.end(), [](Edge& a, Edge& b){
      return a.dst < b.dst;
    });

    sort(Nedges.begin(), Nedges.end(), [](Edge& a, Edge& b){
      return a.dst < b.dst;
    });

    if (Nedges == src_edges){
      return;
    }
    else{
      adjList[msg->from] = Nedges;
      this->sendNbrData(msg->from, msg);
      return;
    }
  }
}



