/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018
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
 * Author: ZhangminWang
 */

#include "mygym.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/wifi-module.h"
#include "ns3/node-list.h"
#include "ns3/log.h"
#include <sstream>
#include <iostream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MyGymEnv");

NS_OBJECT_ENSURE_REGISTERED (MyGymEnv);

MyGymEnv::MyGymEnv ()
{
  NS_LOG_FUNCTION (this);
  m_interval = Seconds (0.1);

  Simulator::Schedule (Seconds (0.0), &MyGymEnv::ScheduleNextStateRead, this);
}

MyGymEnv::MyGymEnv (Time stepTime)
{
  NS_LOG_FUNCTION (this);
  m_interval = stepTime;

  Simulator::Schedule (Seconds (0.0), &MyGymEnv::ScheduleNextStateRead, this);
}

void
MyGymEnv::ScheduleNextStateRead ()
{
  NS_LOG_FUNCTION (this);
  Simulator::Schedule (m_interval, &MyGymEnv::ScheduleNextStateRead, this);
  // Notify();
  Step ();
}

MyGymEnv::~MyGymEnv ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
MyGymEnv::GetTypeId (void)
{
  static TypeId tid = TypeId ("MyGymEnv")
                          .SetParent<OpenGymEnv> ()
                          .SetGroupName ("OpenGym")
                          .AddConstructor<MyGymEnv> ();
  return tid;
}

void
MyGymEnv::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

/*
Define observation space
*/
Ptr<OpenGymSpace>
MyGymEnv::GetObservationSpace (uint32_t id)
{
  // uint32_t nodeNum = 5;
  float low = 0.0;
  float high = 10.0;
  std::vector<uint32_t> shape = {1};
  std::string dtype = TypeNameGet<uint32_t> ();

  Ptr<OpenGymBoxSpace> box = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);

  NS_LOG_UNCOND ("ID " << id << " MyGetObservationSpace: " << box);
  return box;
}

/*
Define action space
*/
Ptr<OpenGymSpace>
MyGymEnv::GetActionSpace (uint32_t id)
{
  Ptr<OpenGymDiscreteSpace> discrete = CreateObject<OpenGymDiscreteSpace> (5);

  NS_LOG_UNCOND ("ID " << id << " MyGetActionSpace: " << discrete);
  return discrete;
}

/*
Define game over condition
*/
bool
MyGymEnv::GetDone (uint32_t id)
{
  bool isGameOver = false;
  bool test = false;
  static float stepCounter = 0.0;
  stepCounter += 1;
  if (stepCounter == 10 && test)
    {
      isGameOver = true;
    }
  NS_LOG_UNCOND ("ID " << id << " MyGetGameOver: " << isGameOver);
  return isGameOver;
}

/*
Collect observations
*/
Ptr<OpenGymDataContainer>
MyGymEnv::GetObservation (uint32_t id)
{
  uint32_t low = 0.0;
  uint32_t high = 10.0;
  Ptr<UniformRandomVariable> rngInt = CreateObject<UniformRandomVariable> ();

  std::vector<uint32_t> shape = {1};
  Ptr<OpenGymBoxContainer<uint32_t>> box = CreateObject<OpenGymBoxContainer<uint32_t>> (shape);

  // generate random data
  uint32_t value = rngInt->GetInteger (low, high);
  box->AddValue (value);

  Ptr<OpenGymTupleContainer> data = CreateObject<OpenGymTupleContainer> ();
  data->Add (box);

  // Print data from tuple
  Ptr<OpenGymBoxContainer<uint32_t>> mbox =
      DynamicCast<OpenGymBoxContainer<uint32_t>> (data->Get (0));
  NS_LOG_UNCOND ("ID " << id << " MyGetObservation: " << data);
  NS_LOG_UNCOND ("---" << mbox);

  return data;
}

/*
Define reward function
*/
float
MyGymEnv::GetReward (uint32_t id)
{
  static float reward = 0.0;
  reward += 1;
  return reward;
}

/*
Define extra info. Optional
*/
std::string
MyGymEnv::GetInfo (uint32_t id)
{
  std::string myInfo = "testInfo";
  NS_LOG_UNCOND ("ID " << id << " MyGetExtraInfo: " << myInfo);
  return myInfo;
}

/*
Execute received actions
*/
bool
MyGymEnv::ExecuteActions (uint32_t id, Ptr<OpenGymDataContainer> action)
{
  Ptr<OpenGymDiscreteContainer> discrete = DynamicCast<OpenGymDiscreteContainer> (action);

  NS_LOG_UNCOND ("ID " << id << " MyExecuteActions: " << discrete);
  return true;
}

} // namespace ns3