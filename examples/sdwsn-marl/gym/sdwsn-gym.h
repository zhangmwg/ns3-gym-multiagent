/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2019
 * 
 * This is Multi-Agent Reinforcement Learning Environment for 
 * Software-Defined Wireless Sensor Networks
 * 
 * Author: Zhangmin Wang
 */ 

#ifndef SDWSN_GYM_H
#define SDWSN_GYM_H

#include "ns3/opengym-module.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/timer.h"
#include "ns3/node.h"

#include <vector>

namespace ns3{

class Packet;

class SinkGymEnv : public OpenGymEnv
{
public:
  SinkGymEnv ();
  virtual ~SinkGymEnv ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  // Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const;

  void SetNodeId (uint32_t id);

  // OpenGym interface
  virtual Ptr<OpenGymSpace> GetObservationSpace () = 0;
  virtual Ptr<OpenGymDataContainer> GetObservation () = 0;
  virtual Ptr<OpenGymSpace> GetActionSpace ();
  virtual bool GetGameOver ();
  virtual float GetReward ();
  virtual std::string GetExtraInfo ();
  virtual bool ExecuteActions (Ptr<OpenGymDataContainer> action);

  // WSN routing control interface

  // sleeping control interface

  // optional functions used to collect obs

protected:
  void DoInitialize (void);

protected:

  /// hello timer
  Timer m_node_htimer;
  Timer m_sink_htimer;
  /// Schedule next send of hello message
  void HelloTimerExpire ();

  /// Provides uniform random variables
  Ptr<UniformRandomVariable> m_uniformRandomVariable;

  // uint32_t m_nodeId;
  uint32_t m_numNode;
  uint32_t m_numSurvive;

  // state, obs have to be implemented in child class
  
  // game over
  bool m_isGameOver;

  // reward
  float m_envReward;

  // extra info
  std::string m_info;

  // actions
  // Ipv4Address nextHop; 

private:
  ObjectFactory m_agentFactory;
};

class SinkEventGymEnv : public SinkGymEnv
{
public:
  SinkEventGymEnv ();
  virtual ~SinkEventGymEnv ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  void SetReward (float value);

  // OpenGym interface
  virtual Ptr<OpenGymSpace> GetObservationSpace ();
  Ptr<OpenGymDataContainer> GetObservation ();

private:
  // state 

  // reward
  float m_reward;
};

class SinkTimeStepGymEnv : public SinkGymEnv
{
public:
  SinkTimeStepGymEnv ();
  SinkTimeStepGymEnv (Time timeStep);
  virtual ~SinkTimeStepGymEnv ();
  static TypeId GetTypeId (void);
  virtual void DoDispose();

  // OpenGym interface
  virtual Ptr<OpenGymSpace> GetObservationSpace ();
  Ptr<OpenGymDataContainer> GetObservation ();

  // optional function used to collect obs

private:
  void ScheduleNextStateRead();
  bool m_started {false};
  Time m_timeStep;
  // state
};

} // namespace

#endif