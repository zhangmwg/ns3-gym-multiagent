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
 * Author: ZhangminWang
 */


#ifndef MY_GYM_ENTITY_H
#define MY_GYM_ENTITY_H

#include "ns3/opengym-module.h"
#include "ns3/nstime.h"

namespace ns3 {

/**
 * \note This is only a test, not use algorithm
 */ 
class MyGymEnv : public OpenGymMultiEnv
{
public:
  MyGymEnv ();
  MyGymEnv (Time stepTime);
  virtual ~MyGymEnv ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();

  Ptr<OpenGymSpace> GetActionSpace(uint32_t id);
  Ptr<OpenGymSpace> GetObservationSpace(uint32_t id);
  bool GetDone(uint32_t id);
  Ptr<OpenGymDataContainer> GetObservation(uint32_t id);
  float GetReward(uint32_t id);
  std::string GetInfo(uint32_t id);
  bool ExecuteActions(uint32_t id, Ptr<OpenGymDataContainer> action);

private:
  void ScheduleNextStateRead();

  Time m_interval;
};

}


#endif // MY_GYM_ENTITY_H
