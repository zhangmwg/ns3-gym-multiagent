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
#include "wsn-packet.h"
#include "ns3/address-utils.h"
#include "ns3/packet.h"

namespace ns3 {
namespace sdwsn {

NS_OBJECT_ENSURE_REGISTERED (TypeHeader);

TypeHeader::TypeHeader (MessageType t)
  : m_type (t),
    m_valid (true)
{
}

TypeId
TypeHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::sdwsn::TypeHeader")
    .SetParent<Header> ()
    .SetGroupName ("Sdwsn")
    .AddConstructor<TypeHeader> ()
  ;
  return tid;
}

TypeId
TypeHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t 
TypeHeader::GetSerializedSize () const
{
  return 1;
}

void TypeHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 ((uint8_t) m_type);
}

uint32_t
TypeHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  uint8_t type = i.ReadU8 ();
  m_valid = true;
  switch (type)
    {
      case E3RTYPE_RREP:
        {
          m_type = (MessageType) type;
          break;
        }
      default:
        m_valid = false;
    }
  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void 
TypeHeader::Print (std::ostream &os) const
{
  switch (m_type)
  {
  case E3RTYPE_RREP:
    {
      os << "RREP";
      break;
    }
  
  default:
    os << "UNKNOWN_TYPE";
  }
}

bool
TypeHeader::operator== (TypeHeader const & o) const
{
  return (m_type == o.m_type && m_valid == o.m_valid);
}

std::ostream &
operator<< (std::ostream & os, TypeHeader const & h)
{
  h.Print (os);
  return os;
}

//-------------------------------------------------
// RREP ( and HELLO )
//-------------------------------------------------
RrepHeader::RrepHeader (uint8_t hopCount, Ipv4Address dst,
                        uint32_t dstSeqNo, Ipv4Address origin)
  : m_hopCount (hopCount),
    m_dst (dst),
    m_dstSeqNo (dstSeqNo),
    m_origin (origin)
{
}

NS_OBJECT_ENSURE_REGISTERED (RrepHeader);

TypeId
RrepHeader::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::sdwsn::RrepHeader")
    .SetParent<Header> ()
    .SetGroupName ("Sdwsn")
    .AddConstructor<RrepHeader> ()
  ;
  return tid;
}

TypeId
RrepHeader::GetInstanceTypeId () const
{
  return GetTypeId ();
}

uint32_t
RrepHeader::GetSerializedSize () const
{
  return 19;
}

void
RrepHeader::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (m_hopCount);
  WriteTo (i, m_dst);
  i.WriteHtonU32 (m_dstSeqNo);
  WriteTo (i, m_origin);
}

uint32_t 
RrepHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;

  m_hopCount = i.ReadU8 ();
  ReadFrom (i, m_dst);
  m_dstSeqNo = i.ReadNtohU32 ();
  ReadFrom (i, m_origin);

  uint32_t dist = i.GetDistanceFrom (start);
  NS_ASSERT (dist == GetSerializedSize ());
  return dist;
}

void 
RrepHeader::Print (std::ostream &os) const
{
  os << "destination ipv4 " << m_dst << " sequence number " << m_dstSeqNo;
  os << " source ipv4 " << m_origin;
}


bool
RrepHeader::operator== (RrepHeader const & o) const
{
  return (m_hopCount == o.m_hopCount && m_dst == o.m_dst
          && m_dstSeqNo == o.m_dstSeqNo && m_origin == o.m_origin);
}

void
RrepHeader::SetHello (Ipv4Address orign, uint32_t srcSeqNo)
{
  m_hopCount = 0;
  m_dst = orign;
  m_dstSeqNo = srcSeqNo;
  m_origin = orign;
}

std::ostream & 
operator << (std::ostream & os, RrepHeader & h)
{
  h.Print (os);
  return os;
}

} // namespace sdwsn
} // namespace ns3