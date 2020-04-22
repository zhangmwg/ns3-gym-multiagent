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
 * 
 * Author: Zhangmin Wang
 */

#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/uinteger.h"
#include "opengym_multi_env.h"
#include "container.h"
#include "spaces.h"
#include "opengym_multi_interface.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (OpenGymMultiEnv);

NS_LOG_COMPONENT_DEFINE ("OpenGymMultiEnv");

TypeId
OpenGymMultiEnv::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::OpenGymMultiEnv")
          .SetParent<Object> ()
          .SetGroupName ("OpenGym")
          .AddAttribute ("OpenGymPort", "OpenGymPort, default 5555", UintegerValue (5555),
                         MakeUintegerAccessor (&OpenGymMultiEnv::m_openGymPort),
                         MakeUintegerChecker<uint32_t> ());
  return tid;
}

OpenGymMultiEnv::OpenGymMultiEnv ()
{
  NS_LOG_FUNCTION (this);
  // Env automatically associate Interface
  Ptr<OpenGymMultiInterface> openGymMultiInterface =
      CreateObject<OpenGymMultiInterface> (m_openGymPort);
  SetOpenGymMultiInterface (openGymMultiInterface);
}

OpenGymMultiEnv::~OpenGymMultiEnv ()
{
  NS_LOG_FUNCTION (this);
}

void
OpenGymMultiEnv::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
}

void
OpenGymMultiEnv::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
}

void
OpenGymMultiEnv::AddAgentId (uint32_t agent_id)
{
  NS_LOG_FUNCTION (this);
  m_openGymMultiInterface->AddAgent (agent_id);
}

void
OpenGymMultiEnv::SetOpenGymMultiInterface (Ptr<OpenGymMultiInterface> multiInterface)
{
  NS_LOG_FUNCTION (this);
  m_openGymMultiInterface = multiInterface;
  multiInterface->SetGetActionSpaceCb (MakeCallback (&OpenGymMultiEnv::GetActionSpace, this));
  multiInterface->SetGetObservationSpaceCb (
      MakeCallback (&OpenGymMultiEnv::GetObservationSpace, this));
  multiInterface->SetGetObservationCb (MakeCallback (&OpenGymMultiEnv::GetObservation, this));
  multiInterface->SetGetRewardCb (MakeCallback (&OpenGymMultiEnv::GetReward, this));
  multiInterface->SetGetDoneCb (MakeCallback (&OpenGymMultiEnv::GetDone, this));
  multiInterface->SetGetInfoCb (MakeCallback (&OpenGymMultiEnv::GetInfo, this));
  multiInterface->SetExecuteActionsCb (MakeCallback (&OpenGymMultiEnv::ExecuteActions, this));
}

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
 */
void
OpenGymMultiEnv::Step ()
{
  NS_LOG_FUNCTION (this);
  if (m_openGymMultiInterface)
    {
      m_openGymMultiInterface->Notify (this);
    }
}

void
OpenGymMultiEnv::NotifySimulationEnd ()
{
  NS_LOG_FUNCTION (this);
  if (m_openGymMultiInterface)
    {
      m_openGymMultiInterface->NotifySimulationEnd ();
    }
}

} // namespace ns3