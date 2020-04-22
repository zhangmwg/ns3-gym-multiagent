/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * 
 * ********************************************************************************
 * 
 * Multi-agent interface, different from single-agent interface (opengym_interface). 
 * We formalize the network problem as a multi-agent extension Markov decision 
 * processes (MDPs) called Partially Observable Markov Games (POMGs).
 *
 * Base on: 
 *    opengym_interface
 * 
 * Author: Zhangmin Wang
 */

#ifndef OPENGYM_MULTI_INTERFACE_H
#define OPENGYM_MULTI_INTERFACE_H

#include "ns3/object.h"
#include <zmq.hpp>

namespace ns3 {

class OpenGymSpace;
class OpenGymDataContainer;
class OpenGymMultiEnv;

/**
 * \note This class should only be called by OpenGymMultiEnv.
 */
class OpenGymMultiInterface : public Object
{
public:
  static Ptr<OpenGymMultiInterface> Get (uint32_t port = 5555);

  OpenGymMultiInterface (uint32_t port = 5555);
  virtual ~OpenGymMultiInterface ();

  static TypeId GetTypeId ();

  void Init ();

  /**
   * \note Notify, similar openAI Gym step
   * Notify network environment current state, including each agent. 
   * Set Get Callback for one agent by agent_id: [observation, reward, done, info, execute_action]
   * Notify current state of agents
   */
  void Notify (Ptr<OpenGymMultiEnv> entity);
  /**
   * Notify current state of agents
   * Send env state msg to python
   * Receive multi-agent actions msg from python
   * 
   * NOTE: first step after reset is called without actions, 
   * just to get current state
   * 
   * 1. Collect current env state
   * 2. Execute Actions
   * \note Similar gym step, this function should only be called by Notify.
   */
  void NotifyCurrentState ();
  void WaitForStop ();
  void NotifySimulationEnd ();

  void AddAgent(uint32_t agent_id);

  // Each agent
  Ptr<OpenGymSpace> GetActionSpace (uint32_t agent_id);
  Ptr<OpenGymSpace> GetObservationSpace (uint32_t agent_id);
  Ptr<OpenGymDataContainer> GetObservation (uint32_t agent_id);
  float GetReward (uint32_t agent_id);
  bool GetDone (uint32_t agent_id);
  std::string GetInfo (uint32_t agent_id);
  bool ExecuteActions (uint32_t agent_id, Ptr<OpenGymDataContainer> action);

  // Each agent OpenGym Interface Callback
  void SetGetActionSpaceCb (Callback<Ptr<OpenGymSpace>, uint32_t> cb);
  void SetGetObservationSpaceCb (Callback<Ptr<OpenGymSpace>, uint32_t> cb);
  void SetGetObservationCb (Callback<Ptr<OpenGymDataContainer>, uint32_t> cb);
  void SetGetRewardCb (Callback<float, uint32_t> cb);
  void SetGetDoneCb (Callback<bool, uint32_t> cb);
  void SetGetInfoCb (Callback<std::string, uint32_t> cb);
  void SetExecuteActionsCb (Callback<bool, uint32_t, Ptr<OpenGymDataContainer>> cb);

protected:
  // Inherited
  virtual void DoInitialize (void);
  virtual void DoDispose (void);

private:
  static Ptr<OpenGymMultiInterface> *DoGet (uint32_t port = 5555);
  static void Delete (void);

  uint32_t m_port;
  zmq::context_t m_zmq_context;
  zmq::socket_t m_zmq_socket;

  bool m_simEnd;
  bool m_stopEnvRequested;
  bool m_initSimMsgSent;
  
  // agent ID vector
  std::vector<uint32_t> m_agentIdVec;

  Callback<Ptr<OpenGymSpace>, uint32_t> m_actionSpaceCb;
  Callback<Ptr<OpenGymSpace>, uint32_t> m_observationSpaceCb;
  Callback<Ptr<OpenGymDataContainer>, uint32_t> m_obsCb;
  Callback<float, uint32_t> m_rewardCb;
  Callback<bool, uint32_t> m_doneCb;
  Callback<std::string, uint32_t> m_infoCb;
  Callback<bool, uint32_t, Ptr<OpenGymDataContainer>> m_actionCb;
};

} // namespace ns3

#endif /* OPENGYM_MULTI_INTERFACE_H */