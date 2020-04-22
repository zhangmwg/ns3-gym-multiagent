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
#include "base-routing-protocol.h"
#include "ns3/log.h"
#include "ns3/boolean.h"
#include "ns3/random-variable-stream.h"
#include "ns3/inet-socket-address.h"
#include "ns3/wifi-net-device.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include <algorithm>
#include <limits>

namespace ns3{

NS_LOG_COMPONENT_DEFINE ("BaseRouting");

namespace sdwsn{
  NS_OBJECT_ENSURE_REGISTERED (BaseRouting);

  const uint32_t BaseRouting::WSN_PORT = 12345;

///////////////////////////////////////////////////////////////////
/**
 * Initialization Phase
 * 
 * Prior to the start of the SDN mode, the controller has to learn inforcement 
 * such as the node distribution and the initial energy of all the nodes in the 
 * network.
 * In this phase, the controller and the sensor nodes will start a routing 
 * discovery procedure, which is against but necessary for the paradigm of 
 * SD-WSNs. 
 */ 


/**
 * In the sensor nodes, the main purpose of routing discovery is to find the 
 * neighbors and fill the neighbors table. Sensor node periodically broadcasts 
 * HELLO (Hello RREQ) packet and waits for the RREP packets from its neighbors. 
 * The sensor nodes fill their neighbor tables with the inforcement obtained 
 * from the RREP packet.
 * 
 * One the sensor nodes have receiver the ROU_RREQ packet from the controller 
 * (i.e., Sink) , the sensor node will packet the neighbor information (neighbor 
 * node address, neighbor node distance) together with the node inforcement 
 * (residual energy, and so on) into ROU_RREP packets.
 * 
 * The ROU_RREP packets will be sent to the controller along the channel from 
 * which the ROU_RREQ packet comes from, and the ROU_RREQ will be broadcast 
 * to their neighbors.
 * 
 */

void
BaseRouting::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  uint32_t startTime;
  m_node_htimer.SetFunction (&BaseRouting::HelloTimerExpire, this);
  startTime = m_uniformRandomVariable->GetInteger (0, 100);
  NS_LOG_DEBUG ("Start at time " << startTime << "ms");
  m_node_htimer.Schedule (MilliSeconds (startTime));

  Ipv4RoutingProtocol::DoInitialize ();
}

void 
BaseRouting::HelloTimerExpire ()
{
  NS_LOG_FUNCTION (this);
  SendHello ();
  m_node_htimer.Cancel ();
}

void 
BaseRouting::SendHello ()
{
  NS_LOG_FUNCTION (this);

  for (std::map<Ptr<Socket>, Ipv4InterfaceAddress>::const_iterator j = m_socketAddresses.begin(); 
       j != m_socketAddresses.end(); ++j)
    {
      Ptr<Socket> socket = j->first;
      Ipv4InterfaceAddress iface = j->second;
      RrepHeader helloHeader (0, iface.GetLocal (), m_seqNo, 
                              iface.GetLocal ());
      Ptr<Packet> packet = Create<Packet> ();
      SocketIpTtlTag tag;
      tag.SetTtl (1);
      packet->AddPacketTag (tag);
      packet->AddHeader (helloHeader);
      TypeHeader tHeader (E3RTYPE_RREP);
      packet->AddHeader (tHeader);
      // Send to all-hosts broadcast
      Ipv4Address destination;
      if (iface.GetMask () == Ipv4Mask::GetOnes ())
        {
          destination = Ipv4Address ("255.255.255.255");
        }
      else
        {
          destination = iface.GetBroadcast ();
        }
      Time jitter = Time (MilliSeconds (m_uniformRandomVariable->GetInteger (0, 10)));
      Simulator::Schedule (jitter, &BaseRouting::SendTo, this, socket, packet, destination);
    }
}

void 
BaseRouting::SendReply (RreqHeader const & rreqHeader, RoutingTableEntry const & toOrigin)
{
  NS_LOG_FUNCTION (this << toOrigin.GetDestination ());
  /*
   * Destination node MUST increment its own sequence number by one if the sequence number in the RREQ packet is equal to that
   * incremented value. Otherwise, the destination does not change its sequence number before generating the  RREP message.
   */
  if (!rreqHeader.GetUnknownSeqno () && (rreqHeader.GetDstSeqno ()) == m_seqNo + 1)
    {
      m_seqNo++;
    }

  /////
  //
  //
  //
  //
}

void 
BaseRouting::SendTo (Ptr<Socket> socket, Ptr<Packet> packet, Ipv4Address destination)
{
  socket->SendTo (packet, 0, InetSocketAddress (destination, WSN_PORT));
}

// void E3RoutingProtocol::SendRequest (Ipv4Address dst)
// {
//   NS_LOG_FUNCTION (this << dst);
//   uint32_t m_rreqCount;
  
//   m_rreqCount++;
//   // Create RREQ header
//   Rreqheader rreqHeader;
//   rreqHeader.SetDst (dst);

//   RoutingTableEntry rt;
//   // Using the Hop field 
//   if (m_routingTable.LookupRoute (dst, rt))
//     {
//       if (rt.GetValidSeqNo())
//         {
//           rreqHeader.SetDstSeqNo (rt.GetSeqNo());
//         }
//       else
//         {
//           rreqHeader.SetUnknownSeqno(true);
//         }
//       rt.SetHop ();
//       rt.SetFlag (IN_SEARCH);
//       // rt.SetLifeTime ();
//       m_routingtable.Update (rt);
//     }
//   else
//   {
//     rreqHeader.SetUnknownSeqno (true);
//     Ptr<NetDevice> dev = 0;
//     RoutingTableEntry newEntry ();
//   }
  
// }

/**
 * In the controller, the main purpose of routing discovery is to obtain 
 * the network interconnection map (show in an adjacency martix) and other 
 * information (link quality, residual energy). 
 * 
 * Base on such information, the controller (i.e., Sink) can flexibly generate
 * forwarding rules through Reinforcement Learning Algorithm.
 * 
 * A preset time after the start of the network (to ensure that the sensor 
 * have found their neighbors), the contriller broadcasts ROU_RREQ packet and
 * waits for a period of time to receive the ROU_RREP packets. 
 * 
 * During the routing discovery period, the controller sends the inforcement in
 * in the ROU_RREP packet as long as it receives a ROU_RREP from a sensor node.
 * 
 * After a fixed amount of time, the forwarding rules will be generated based 
 * on the Reinforcement Learning algorithm by Controller's Agent in Sink. 
 */ 


///////////////////////////////////////////////////////////////////
/**
 * Maintenance Phase
 * 
 * In the maintenance phase, the main task is to adjust the forwarding rules 
 * according to change in the network. The processes in the controller and
 * the sensor nodes are also different.
 */ 

/**
 * In the sensor node, the main purpose of mainteance is to detect changes in 
 * neighbor relains.
 * 
 * After sending the ROU_RREP to the controller, the senosr nodes start
 * the maintenance procedure. The controller will periodically broadcast
 * HELLO_RREQ packets to collect the neighbor inforcement.
 * 
 * Prior to the next broadcast of HELLO_RREQ, the sensor nodes compare the 
 * neighbor information collected during the last two HELLO_RREQ periods, and
 * the residual energy will be detected.
 * 
 * If any differences are detected or the residual energy reaches the threshold,
 * the sensor nodes packet the change inforcement into ROU_REPAIR packets and 
 * send them to the controller.
 */ 

/**
 * In the controller, after the routing discovery, the controller continues 
 * waiting until it has received a ROU_REPAIR packet or a new ROU_RREP packet.
 * Then, the controller modifies the forwarding rules in the Experience replay
 * according to the changed routing state.
 */ 

} // namespace sdwsn
} // namespace ns3