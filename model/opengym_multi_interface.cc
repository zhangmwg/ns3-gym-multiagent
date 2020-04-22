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

#include <sys/types.h>
#include <unistd.h>
#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "opengym_multi_interface.h"
#include "opengym_multi_env.h"
#include "container.h"
#include "spaces.h"
#include "messages.pb.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("OpenGymMultiInterface");

NS_OBJECT_ENSURE_REGISTERED (OpenGymMultiInterface);

TypeId
OpenGymMultiInterface::GetTypeId (void)
{
  static TypeId tid = TypeId ("OpenGymMultiInterface")
                          .SetParent<Object> ()
                          .SetGroupName ("OpenGym")
                          .AddConstructor<OpenGymMultiInterface> ();
  return tid;
}

Ptr<OpenGymMultiInterface>
OpenGymMultiInterface::Get (uint32_t port)
{
  NS_LOG_FUNCTION_NOARGS ();
  return *DoGet (port);
}

Ptr<OpenGymMultiInterface> *
OpenGymMultiInterface::DoGet (uint32_t port)
{
  NS_LOG_FUNCTION_NOARGS ();
  static Ptr<OpenGymMultiInterface> ptr = 0;
  if (ptr == 0)
    {
      ptr = CreateObject<OpenGymMultiInterface> (port);
      Config::RegisterRootNamespaceObject (ptr);
      Simulator::ScheduleDestroy (&OpenGymMultiInterface::Delete);
    }
  return &ptr;
}

void
OpenGymMultiInterface::Delete (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Config::UnregisterRootNamespaceObject (Get ());
  (*DoGet ()) = 0;
}

OpenGymMultiInterface::OpenGymMultiInterface (uint32_t port)
    : m_port (port),
      m_zmq_context (1),
      m_zmq_socket (m_zmq_context, ZMQ_REQ),
      m_simEnd (false),
      m_stopEnvRequested (false),
      m_initSimMsgSent (false)
{
  NS_LOG_FUNCTION (this);
}

OpenGymMultiInterface::~OpenGymMultiInterface ()
{
  NS_LOG_FUNCTION (this);
}

void
OpenGymMultiInterface::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
}

void
OpenGymMultiInterface::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
}

void
OpenGymMultiInterface::SetGetActionSpaceCb (Callback<Ptr<OpenGymSpace>, uint32_t> cb)
{
  NS_LOG_FUNCTION (this);
  m_actionSpaceCb = cb;
}

void
OpenGymMultiInterface::SetGetObservationSpaceCb (Callback<Ptr<OpenGymSpace>, uint32_t> cb)
{
  NS_LOG_FUNCTION (this);
  m_observationSpaceCb = cb;
}

void
OpenGymMultiInterface::SetGetObservationCb (Callback<Ptr<OpenGymDataContainer>, uint32_t> cb)
{
  NS_LOG_FUNCTION (this);
  m_obsCb = cb;
}

void
OpenGymMultiInterface::SetGetRewardCb (Callback<float, uint32_t> cb)
{
  NS_LOG_UNCOND (this);
  m_rewardCb = cb;
}

void
OpenGymMultiInterface::SetGetDoneCb (Callback<bool, uint32_t> cb)
{
  NS_LOG_UNCOND (this);
  m_doneCb = cb;
}

void
OpenGymMultiInterface::SetGetInfoCb (Callback<std::string, uint32_t> cb)
{
  NS_LOG_UNCOND (this);
  m_infoCb = cb;
}

void
OpenGymMultiInterface::SetExecuteActionsCb (Callback<bool, uint32_t, Ptr<OpenGymDataContainer>> cb)
{
  NS_LOG_FUNCTION (this);
  m_actionCb = cb;
}

void
OpenGymMultiInterface::Init ()
{
  NS_LOG_FUNCTION (this);
  // do not send init msg twice
  if (m_initSimMsgSent)
    {
      return;
    }
  if (m_agentIdVec.size () == 0)
    {
      NS_LOG_UNCOND ("\n-------------------------ERROR--------------------------------");
      NS_LOG_UNCOND ("> ERROR! Please check if AddAgent() is called in ENV!");
      NS_LOG_UNCOND ("Agent vector size: " << m_agentIdVec.size ());
      NS_LOG_UNCOND ("You should first use ENV->AddAgent().");
      NS_LOG_UNCOND ("> ERROR ... [Please Key Ctrl+C]");
      return;
    }
  m_initSimMsgSent = true;

  std::string connectAddr = "tcp://localhost:" + std::to_string (m_port);
  zmq_connect ((void *) m_zmq_socket, connectAddr.c_str ());

  NS_LOG_UNCOND ("\nAgent vector size: " << m_agentIdVec.size ());

  ns3opengym::MultiAgentInitMsg multiAgentInitMsg;
  multiAgentInitMsg.set_simprocessid (::getpid ());
  multiAgentInitMsg.set_wafshellprocessid (::getppid ());

  for (std::vector<uint32_t>::const_iterator i = m_agentIdVec.begin (); i != m_agentIdVec.end ();
       i++)
    {
      uint32_t agent_id = *i;
      Ptr<OpenGymSpace> obsSpace = GetObservationSpace (agent_id);
      Ptr<OpenGymSpace> actionSpace = GetActionSpace (agent_id);

      ns3opengym::AgentInitMsg *agentInitMsg;
      agentInitMsg = multiAgentInitMsg.add_agentinitmsg ();
      // agent ID
      agentInitMsg->set_agentid (agent_id);
      if (obsSpace)
        {
          ns3opengym::SpaceDescription spaceDecs;
          spaceDecs = obsSpace->GetSpaceDescription ();
          agentInitMsg->mutable_obsspace ()->CopyFrom (spaceDecs);
        }
      if (actionSpace)
        {
          ns3opengym::SpaceDescription spaceDecs;
          spaceDecs = actionSpace->GetSpaceDescription ();
          agentInitMsg->mutable_actspace ()->CopyFrom (spaceDecs);
        }
    }
  NS_LOG_UNCOND ("\n=============================================================================");
  NS_LOG_UNCOND ("\nSimulation process id: " << ::getpid ()
                                             << " (parent (waf shell) id: " << ::getppid () << ")");
  NS_LOG_UNCOND ("Waiting for Python process to connect on port: " << connectAddr);
  NS_LOG_UNCOND ("Please start proper Python AI Agent ...\n");

  // send init msg to python
  zmq::message_t request (multiAgentInitMsg.ByteSize ());
  multiAgentInitMsg.SerializeToArray (request.data (), multiAgentInitMsg.ByteSize ());
  m_zmq_socket.send (request);

  // receive init ack msg from python
  ns3opengym::SimInitAck simInitAck;
  zmq::message_t reply;
  m_zmq_socket.recv (&reply);
  simInitAck.ParseFromArray (reply.data (), reply.size ());

  bool done = simInitAck.done ();
  NS_LOG_DEBUG ("Sim Init Ack: " << done);
  bool stopSim = simInitAck.stopsimreq ();
  if (stopSim)
    {
      NS_LOG_DEBUG ("--Stop requested: " << stopSim);
      m_stopEnvRequested = true;
      Simulator::Stop ();
      Simulator::Destroy ();
      std::exit (0);
    }
}

/**
 * Notify network environment current state, including each agent
 */
void
OpenGymMultiInterface::NotifyCurrentState ()
{
  NS_LOG_FUNCTION (this);

  if (!m_initSimMsgSent)
    {
      Init ();
    }

  if (m_stopEnvRequested)
    {
      return;
    }

  // collect current env state
  ns3opengym::MultiAgentStateMsg multiAgentStateMsg;

  for (std::vector<uint32_t>::const_iterator i = m_agentIdVec.begin (); i != m_agentIdVec.end ();
       i++)
    {
      uint32_t agent_id = *i;
      Ptr<OpenGymDataContainer> obsDataContainer = GetObservation (agent_id);
      float reward = GetReward (agent_id);
      bool done = GetDone (agent_id);
      std::string info = GetInfo (agent_id);

      ns3opengym::AgentStateMsg *agentStateMsg;
      agentStateMsg = multiAgentStateMsg.add_agentstatemsg ();
      // agent ID
      agentStateMsg->set_agentid (agent_id);
      // observation
      ns3opengym::DataContainer obsDataContainerPbMsg;
      if (obsDataContainer)
        {
          obsDataContainerPbMsg = obsDataContainer->GetDataContainerPbMsg ();
          agentStateMsg->mutable_obsdata ()->CopyFrom (obsDataContainerPbMsg);
        }
      // reward
      agentStateMsg->set_reward (reward);
      // done
      agentStateMsg->set_done (false);
      if (done)
        {
          agentStateMsg->set_done (true);
        }
      // info
      agentStateMsg->set_info (info);
    }

  // send env state msg to python
  zmq::message_t request (multiAgentStateMsg.ByteSize ());
  multiAgentStateMsg.SerializeToArray (request.data (), multiAgentStateMsg.ByteSize ());
  m_zmq_socket.send (request);

  // receive multi-agent actions msg from python
  ns3opengym::MultiAgentActMsg multiAgentActMsg;
  zmq::message_t reply;
  m_zmq_socket.recv (&reply);
  multiAgentActMsg.ParseFromArray (reply.data (), reply.size ());

  if (m_simEnd)
    {
      // if sim end only rx ms and quit
      return;
    }

  bool stopSim = multiAgentActMsg.stopsimreq ();
  if (stopSim)
    {
      NS_LOG_DEBUG ("---Stop requested: " << stopSim);
      m_stopEnvRequested = true;
      Simulator::Stop ();
      Simulator::Destroy ();
      std::exit (0);
    }

  // first step after reset is called without actions, just to get current state
  // execute actions for each agent
  NS_LOG_DEBUG ("multiAgentActMsg.agentactmsg_size " << multiAgentActMsg.agentactmsg_size ());
  for (int i = 0; i < multiAgentActMsg.agentactmsg_size (); i++)
    {
      const ns3opengym::AgentActMsg &agentActMsg = multiAgentActMsg.agentactmsg (i);
      uint32_t agent_id = agentActMsg.agentid ();
      ns3opengym::DataContainer actDataContainerPbMsg = agentActMsg.actdata ();
      Ptr<OpenGymDataContainer> actDataContainer =
          OpenGymDataContainer::CreateFromDataContainerPbMsg (actDataContainerPbMsg);
      NS_LOG_DEBUG ("NotifyCurrentState ExecuteActions"
                    << " agent_id," << agent_id << " actDataContainer," << actDataContainer);
      ExecuteActions (agent_id, actDataContainer);
    }
}

void
OpenGymMultiInterface::WaitForStop ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("Wait for stop message");
  NotifyCurrentState ();
}

void
OpenGymMultiInterface::NotifySimulationEnd ()
{
  NS_LOG_FUNCTION (this);
  m_simEnd = true;
  if (m_initSimMsgSent)
    {
      WaitForStop ();
    }
}

void
OpenGymMultiInterface::AddAgent (uint32_t agent_id)
{
  m_agentIdVec.push_back (agent_id);
}

Ptr<OpenGymSpace>
OpenGymMultiInterface::GetObservationSpace (uint32_t agent_id)
{
  NS_LOG_FUNCTION (this);
  Ptr<OpenGymSpace> obsSpace;
  if (!m_observationSpaceCb.IsNull ())
    {
      obsSpace = m_observationSpaceCb (agent_id);
    }
  return obsSpace;
}

Ptr<OpenGymSpace>
OpenGymMultiInterface::GetActionSpace (uint32_t agent_id)
{
  NS_LOG_FUNCTION (this);
  Ptr<OpenGymSpace> actionSpace;
  if (!m_actionSpaceCb.IsNull ())
    {
      actionSpace = m_actionSpaceCb (agent_id);
    }
  return actionSpace;
}

Ptr<OpenGymDataContainer>
OpenGymMultiInterface::GetObservation (uint32_t agent_id)
{
  NS_LOG_FUNCTION (this);
  Ptr<OpenGymDataContainer> obs;
  if (!m_obsCb.IsNull ())
    {
      obs = m_obsCb (agent_id);
    }
  return obs;
}

float
OpenGymMultiInterface::GetReward (uint32_t agent_id)
{
  NS_LOG_FUNCTION (this);
  float reward = 0.0;
  if (!m_rewardCb.IsNull ())
    {
      reward = m_rewardCb (agent_id);
    }
  return reward;
}

bool
OpenGymMultiInterface::GetDone (uint32_t agent_id)
{
  NS_LOG_FUNCTION (this);
  bool done = false;
  if (!m_doneCb.IsNull ())
    {
      done = m_doneCb (agent_id);
    }
  return done;
}

std::string
OpenGymMultiInterface::GetInfo (uint32_t agent_id)
{
  NS_LOG_FUNCTION (this);
  std::string info;
  if (!m_infoCb.IsNull ())
    {
      info = m_infoCb (agent_id);
    }
  return info;
}

bool
OpenGymMultiInterface::ExecuteActions (uint32_t agent_id, Ptr<OpenGymDataContainer> action)
{
  NS_LOG_FUNCTION (this);
  bool reply = false;
  NS_LOG_DEBUG ("OpenGymMultiInterface::ExecuteActions "
                << "agent_id," << agent_id << " action," << action);
  if (!m_actionCb.IsNull ())
    {
      reply = m_actionCb (agent_id, action);
    }
  return reply;
}

/**
 * Notify, Similar gym step
 * Set Get Callback for one agent by agent_id: [observation, reward, done, info, execute_action]
 * Notify current state of agents
 */
void
OpenGymMultiInterface::Notify (Ptr<OpenGymMultiEnv> entity)
{
  NS_LOG_FUNCTION (this);
  SetGetObservationCb (MakeCallback (&OpenGymMultiEnv::GetObservation, entity));
  SetGetRewardCb (MakeCallback (&OpenGymMultiEnv::GetReward, entity));
  SetGetDoneCb (MakeCallback (&OpenGymMultiEnv::GetDone, entity));
  SetGetInfoCb (MakeCallback (&OpenGymMultiEnv::GetInfo, entity));
  SetExecuteActionsCb (MakeCallback (&OpenGymMultiEnv::ExecuteActions, entity));

  NotifyCurrentState ();
}

} // namespace ns3