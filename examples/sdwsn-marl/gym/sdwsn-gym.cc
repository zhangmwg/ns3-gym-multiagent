/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2019
 * 
 * This is Multi-Agent Reinforcement Learning Environment for 
 * Software-Defined Wireless Sensor Networks
 * 
 * Author: Zhangmin Wang
 */

#include "sdwsn-gym.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include <vector>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("ns3::SinkGymEnv");
NS_OBJECT_ENSURE_REGISTERED (SinkGymEnv);

SinkGymEnv::SinkGymEnv ()
{
  NS_LOG_FUNCTION (this);
  SetOpenGymInterface (OpenGymInterface::Get ());
}

SinkGymEnv::~SinkGymEnv ()
{
  NS_LOG_FUNCTION (this);
}

TypeId SinkGymEnv::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SinkGymEnv")
    .SetParent<OpenGymEnv> ()
    .SetGroupName ("OpenGymEnv")
  ;
  return tid;
}

void SinkGymEnv::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

// Ptr<Ipv4RoutingProtocol>
// SinkGymEnv::Create (Ptr<Node> node) const
// {
//   Ptr<SinkGymEnv> agent = m_agentFactory.Create<SinkGymEnv> ();
//   node->AggregateObject (agent);
//   return agent;
// }

void 
SinkGymEnv::DoInitialize (void)
{

  NS_LOG_FUNCTION (this);
  // Create ()
  uint32_t startTime;
  m_node_htimer.SetFunction (&SinkGymEnv::HelloTimerExpire, this);
  m_sink_htimer.SetFunction (&SinkGymEnv::HelloTimerExpire, this);
  startTime = m_uniformRandomVariable->GetInteger (0, 100);
  NS_LOG_DEBUG ("Sink HELLO start at time " << startTime << "ms");
  m_node_htimer.Schedule (MilliSeconds (startTime));
  m_sink_htimer.Schedule (MilliSeconds (startTime));

  OpenGymEnv::DoInitialize();
}

void SinkGymEnv::SetNodeId (uint32_t id)
{
  NS_LOG_FUNCTION (this);
  // m_nodeId = id;
}

/**
 * Define action space
 */ 
Ptr<OpenGymSpace>
SinkGymEnv::GetActionSpace()
{
  float low = 0.0;
  float high = 65535;
  // nodeId, nextHop
  std::vector<uint32_t> shape = {2};
  std::string dtype = TypeNameGet<uint32_t> ();

  Ptr<OpenGymBoxSpace> space = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
  NS_LOG_INFO ("MyGetActionSpace: " << space);
  return space;
}

/**
 * Define reward function
 */ 
float
SinkGymEnv::GetReward()
{
  NS_LOG_INFO ("MyGetReward: " << m_envReward);
  return m_envReward;
}

/**
 * Execute received actions
 */ 
bool 
SinkGymEnv::ExecuteActions (Ptr<OpenGymDataContainer> action)
{
  Ptr<OpenGymBoxContainer<uint32_t> > box = DynamicCast<OpenGymBoxContainer<uint32_t> >(action);
  uint32_t m_nodeId = box->GetValue(0);
  uint32_t m_nextHop = box->GetValue(1);
  NS_LOG_INFO ("MyExecuteActions: " << action);
  return true;
}


//----------------------------------------------------------
NS_OBJECT_ENSURE_REGISTERED (SinkEventGymEnv);

SinkEventGymEnv::SinkEventGymEnv ()
  : SinkGymEnv()
{
  NS_LOG_FUNCTION (this);
}

SinkEventGymEnv::~SinkEventGymEnv ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
SinkEventGymEnv::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SinkEventGymEnv")
    .SetParent<SinkGymEnv> ()
    .SetGroupName ("OpenGym")
    .AddConstructor<SinkEventGymEnv> ()
  ;
  return tid;
}

void
SinkEventGymEnv::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

void 
SinkEventGymEnv::SetReward (float value)
{
  NS_LOG_FUNCTION (this);
  m_reward = value;
}

/**
 * Define observation space
 */ 
Ptr<OpenGymSpace>
SinkEventGymEnv::GetObservationSpace ()
{
  /**
   * A sensor node receives a CROU_RQ packet from the controller (i.e., Sink),
   * it will packet the Neighbor List information into a CROU_RP packet together 
   * with it's owner information. The CROU_RP packet will be sent back to the 
   * controller along the same route that CROU_RQ comes from.
   * 
   * In the controller, this information from CROU_RP of the each node will 
   * be decode, and after this process is complete, controller form OBSERVATION. 
   * The controller can use the RL method to obtain the secure link from aeach 
   * node to the controller.
   */ 

  // float low = 0.0;
  // float high = 1000.0;
  // std::vector<uint32_t> shape = {3};
  // std::string dtype = TypeNameGet<uint64_t> ();
 
  /**
   * A sensor node name (id) and it's observation when send CROU_RP
   * nodeId, obs: [energy, neighbor serial number]
   * map <std::string node_name, OpenGymSpace node_obs>
   */ 
  Ptr<OpenGymBoxSpace> space = Create<OpenGymBoxSpace> ();

  NS_LOG_INFO ("MyGetObservationSpace: " << space);
  return space;
}

/**
 * Collect observations
 */ 
Ptr<OpenGymDataContainer>
SinkEventGymEnv::GetObservation ()
{
  /**
   * period collect
   * 
   * A sensor node receives a CROU_RQ packet from the controller (i.e., Sink),
   * it will packet the Neighbor List information into a CROU_RP packet together 
   * with it's owner information. The CROU_RP packet will be sent back to the 
   * controller along the same route that CROU_RQ comes from.
   * 
   * In the controller, this information from CROU_RP of the each node will 
   * be decode, and after this process is complete, controller form OBSERVATION. 
   * The controller can use the RL method to obtain the secure link from aeach 
   * node to the controller.
   */ 

  std::string node_name = "node_1";
  
  Ptr<OpenGymDictContainer> space = Create<OpenGymDictContainer> ();
  for(uint32_t i = 0; i < m_numSurvive; i++)
    {
      std::vector<uint32_t> shape = {2};
      Ptr<OpenGymBoxContainer<uint64_t> > node_obs = CreateObject<OpenGymBoxContainer<uint64_t> >(shape);
      uint64_t m_energy;

      // uint64_t m_neighborAddress;

      node_obs->AddValue(m_energy);
      space->Add(node_name, node_obs); 
      
      NS_LOG_INFO ("MyGetObservation: " << space);
      return space;
    }
}


//---------------------------------------------------------
NS_OBJECT_ENSURE_REGISTERED (SinkTimeStepGymEnv);

// SinkTimeStepGymEnv::SinkTimeStepGymEnv ()
//   : SinkGymEnv ()
// {
//   NS_LOG_FUNCTION (this);
//   m_envReward = 0;
// }

// SinkTimeStepGymEnv::SinkTimeStepGymEnv (Time timeStep)
//   : SinkGymEnv ()
// {
//   NS_LOG_FUNCTION (this);
//   m_timeStep = timeStep;
//   m_envReward = 0.0;
// }

// void 
// SinkTimeStepGymEnv::ScheduleNextStateRead ()
// {
//   NS_LOG_FUNCTION (this);
//   Simulator::Schedule (m_timeStep, &SinkTimeStepGymEnv::ScheduleNextStateRead, this);
//   Notify();
// }

// SinkTimeStepGymEnv::~SinkTimeStepGymEnv ()
// {
//   NS_LOG_FUNCTION (this);
// }

// TypeId
// SinkTimeStepGymEnv::GetTypeId (void)
// {
//   static TypeId tid = TypeId ("ns3::SinkTimeStepGymEnv")
//     .SetParent<SinkGymEnv> ()
//     .SetGroupName ("OpenGym")
//     .AddConstructor<SinkTimeStepGymEnv> ()
//   ;
//   return tid;
// }

// void 
// SinkTimeStepGymEnv::DoDispose()
// {
//   NS_LOG_FUNCTION (this);
// }

// /**
//  * Define observation space
//  */ 
// Ptr<OpenGymSpace>
// SinkTimeStepGymEnv::GetObservationSpace ()
// {
//   Ptr<OpenGymBoxSpace> space = Create<OpenGymBoxSpace> ();

//   NS_LOG_INFO ("MyGetObservationSpace: " << space);
//   return space;
// }

// /**
//  * Collect observation
//  */ 
// Ptr<OpenGymDataContainer>
// SinkTimeStepGymEnv::GetObservation ()
// {

// }

} // namespace ns3