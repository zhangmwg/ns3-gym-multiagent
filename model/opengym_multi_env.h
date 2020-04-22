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
 * Multi-agent Environment
 * Base on: 
 *    opengym_env
 * 
 * Author: Zhangmin Wang
 */

/*************************************************************************************
 * NOTE: We formalize the network problem as a multi-agent extension Markov decision 
 * processes (MDPs) called Partially Observable Markov Games (POMGs).
 * 
 * Why we use a multi-agent environment?
 * 
 * Fully-distributed learning:
 * Algorithms with centralized learning process are not applicable 
 * in the real computer network. The centralized learning controller 
 * is usually unable to gather collected environment transitions 
 * from widely distributed routers once an action is executed somewhere 
 * and to update the parameters of each neural network simultaneously 
 * caused by the limited bandwidth.
 * \ref You, Xinyu, et al. "Toward Packet Routing with Fully-distributed 
 * Multi-agent Deep Reinforcement Learning." arXiv preprint 
 * arXiv:1905.03494 (2019).
 *************************************************************************************/ 

#ifndef OPENGYM_MULTI_ENV_H
#define OPENGYM_MULTI_ENV_H

#include "ns3/object.h"

namespace ns3{

class OpenGymSpace;
class OpenGymDataContainer;
class OpenGymMultiInterface;

class OpenGymMultiEnv : public Object
{
public:
  OpenGymMultiEnv();
  virtual ~OpenGymMultiEnv();

  static TypeId GetTypeId();

  // Add agent ID
  void AddAgentId(uint32_t agent_id);

  ///\{ Each agent OpenGym Env 
  virtual Ptr<OpenGymSpace> GetActionSpace(uint32_t agent_id) = 0;
  virtual Ptr<OpenGymSpace> GetObservationSpace(uint32_t agent_id) = 0;
  virtual Ptr<OpenGymDataContainer> GetObservation(uint32_t agent_id) = 0;
  virtual float GetReward(uint32_t agent_id) = 0;
  virtual bool GetDone(uint32_t agent_id) = 0;
  virtual std::string GetInfo(uint32_t agent_id) = 0;
  virtual bool ExecuteActions(uint32_t agent_id, Ptr<OpenGymDataContainer> action) = 0;
  ///\}
  
  /**
   * \brief Notify Current State, similar gym step.
   * 1. Set Callback (SetGetDoneCb,SetGetObservationCb, SetGetRewardCb, 
   *                 SetGetExtraInfoCb, SetExecuteActionsCb)
   * 2. Collect current env state
   * 3. Execute Actions
   * 
   * Notify network environment current state, including each agent. 
   * Set Get Callback for one agent by agent_id: [observation, reward, done, info, execute_action]
   * Notify current state of agents
   * \note Same as in OpenGymEnv::Notify()
   * \note We rename step, because the function is the same as openAI Gym step()
   */
  void Step();
  void NotifySimulationEnd();

protected:
  // Inherited
  virtual void DoInitialize(void);
  virtual void DoDispose(void);
  Ptr<OpenGymMultiInterface> m_openGymMultiInterface;
  uint32_t m_openGymPort = 5555;
private: 
  void SetOpenGymMultiInterface(Ptr<OpenGymMultiInterface> multiInterface);
};

} // namespace ns3

#endif /* OPENGYM_MULTI_ENV */