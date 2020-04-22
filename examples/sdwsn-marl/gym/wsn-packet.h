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
#ifndef WSN_PACKET_H
#define WSN_PACKET_H

#include <iostream>
#include "ns3/header.h"
#include "ns3/enum.h"
#include "ns3/ipv4-address.h"
#include <map>
#include "ns3/nstime.h"

namespace ns3 {
namespace sdwsn {

/**
 * \ingroup sdwsn
 * \brief Message Type enumeration
 */
enum MessageType
{
  // E3RTYPE_HELLO = 1,
  E3RTYPE_RREP = 2,   // Hello packet uses RREP packet format
  E3RTYPE_RERR = 3,
  E3RTYPE_RREP_ACK = 4, 
  E3RTYPE_C_RQ = 5,   // RQ from the controller (i.e., Sink)
  E3RTYPE_C_RP = 6    // RP to the controller
};

/**
 * \ingroup sdwsn
 * \brief types
 */ 
class TypeHeader : public Header
{
public:
  TypeHeader (MessageType t);
  static TypeId GetTypeId ();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  /**
   * \return the type
   */ 
  MessageType Get () const
  {
    return m_type;
  }
  /**
   * Check that type if valid
   * \return true if the type is valid
   */ 
  bool IsValid () const
  {
    return m_valid;
  }

  bool operator== (TypeHeader const & o) const;
private:
  MessageType m_type; ///< type of the message
  bool m_valid; ///< Indicates if the message is valid
};

std::ostream & operator<< (std::ostream & os, TypeHeader const & h);

/**
 * \ingroup sdwsn
 * \brief Route Reply (RREP) Message Format
 * Hello packet uses RREP packet format
 */ 
class RrepHeader : public Header
{
public:
  /**
   * construxtor
   * 
   * //\param prefixSize the prefix size (0)
   * \param hopCount the hop count (0)
   * \param dst the destination sequence number
   * \param origin the origin IP address
   */ 
  RrepHeader (uint8_t hopCount = 0, Ipv4Address = Ipv4Address (),
              uint32_t dstSeqNo = 0, Ipv4Address origin = Ipv4Address ());
  static TypeId GetTypeId();
  TypeId GetInstanceTypeId () const;
  uint32_t GetSerializedSize () const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);
  void Print (std::ostream &os) const;

  // Fields
  void SetHopCount (uint8_t count)
  {
    m_hopCount = count;
  }
  uint8_t GetHopCount () const
  {
    return m_hopCount;
  }
  void SetDst (Ipv4Address a)
  {
    m_dst = a;
  }
  Ipv4Address GetDst () const
  {
    return m_dst;
  }
  void SetDstSeqno (uint32_t s)
  {
    m_dstSeqNo = s;
  }
  uint32_t GetDstSeqno () const
  {
    return m_dstSeqNo;
  }
  void SetOrigin (Ipv4Address a)
  {
    m_origin = a;
  }
  Ipv4Address GetOrigin () const
  {
    return m_origin;
  }

  /**
   * Configure RREP to be a HELLO message
   * 
   * \param src the source IP address
   * \param srcSeqNo the source sequence number
   */ 
  void SetHello (Ipv4Address src, uint32_t srcSeqNo);

  bool operator== (RrepHeader const & o) const;

private:
  // uint8_t m_flags;  ///< A - acknowledgment required flag
  // uint8_t m_prefixSize;
  uint8_t m_hopCount;
  Ipv4Address m_dst;    ///< Destination IP Address
  uint32_t m_dstSeqNo;
  Ipv4Address m_origin; ///< Source IP Address
};

} // namespace sdwsn
} // namespace ns3

#endif