/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2019 
 * 
 * Energy-efficient Routing Using Maximum Entropy Reinforcement Learning 
 * in Software-Defined Wireless Sensor Networks
 * 
 * E3R: Energy-Efficient using maximum Entropy reinforcement learning Routing
 * 
 * This is Routing Helper for E3R base routing (without reinforcement learning)
 * in Software-Defined Wireless Sensor Networks
 *  
 * Author: Zhangmin Wang
 */
#ifndef BASE_ROUTING_PROTOCOL_H
#define BASE_ROUTING_PROTOCOL_H

#include "wsn-packet.h"
#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/timer.h"
#include <map>

namespace ns3 {
namespace sdwsn {

class BaseRouting : public Ipv4RoutingProtocol 
{
public:
  static TypeId GetTypeId (void);
  static const uint32_t WSN_PORT;

  BaseRouting();
  virtual ~BaseRouting();
  virtual void DoDispose ();

  // From Ipv4RoutingProtocol
  Ptr<Ipv4Route> RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, Socket::SocketErrno &sockerr);
  bool RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev, UnicastForwardCallback ucb,
                   MulticastForwardCallback mcb, LocalDeliverCallback lcb, ErrorCallback ecb);
  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void setIpv4 (Ptr<Ipv4> ipv4);

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model. Return the number of stream (possibly zero) that
   * have been assigned.
   * 
   * \param stream first stream index to use
   * \return the number of stream indices assigned by this model
   */ 
  int64_t AssignStreams (int64_t stream);

protected:
  void DoInitialize (void);

private:
  bool m_isSink;

  /// Nodes IP address
  Ipv4Address m_mainAddress;
  /// IP protocol
  Ptr<Ipv4> m_ipv4;
  /// Raw socket per each IP interface, map socket -> iface address (IP + mask)
  std::map<Ptr<Socket>, Ipv4InterfaceAddress> m_socketAddresses;
  /// Loopback device used to defer route requests until a route is found
  Ptr<NetDevice> m_lo;
  /// Main Routing table for the node
  // RoutingTable m_routingTable;

  /// Request sequence number
  uint32_t m_seqNo;

private:
  /// Start protocol operation
  void Start();
  /**
   * Queue packet until we find a route
   * 
   * 类似于 AODV：DeferredRouteOutput， OLSR：QueueMessage。
   */ 
  void EnqueuePacket (Ptr<Packet> p, const Ipv4Header &header);
  /// Find socket with local interface address iface
  Ptr<Socket> FindSocketWithInterfaceAddress (Ipv4InterfaceAddress iface) const;

  /**
   * Send packet to destination socket
   * \param socket - destination node socket
   * \param packet - packet to send
   * \param destination -destination node IP address
   */ 
  void SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination);

  /// hello timer
  Timer m_node_htimer;
  Timer m_sink_htimer;
  /// Schedule next send of hello message
  void HelloTimerExpire ();

  ///\name Send
  //\{
  /// Send Hello
  void SendHello ();
  /// Send RREQ
  void SendRequest (Ipv4Address dst);
  /// Send RREP
  void SendReply (RrepHeader const & rreqHeader, RoutingtableEntry const & toOrigin);
  //\}

  /// Provides uniform random variables
  Ptr<UniformRandomVariable> m_uniformRandomVariable;

};

} // namespace sdwsn
} // namespace ns3

#endif