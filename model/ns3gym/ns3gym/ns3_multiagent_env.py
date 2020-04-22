__author__ = "Zhangmin Wang"
__copyright__ = "Copyright (c) 2019"
__version__ = "0.1.1"
__email__ = "zhangmwg@gmail.com"

"""
MultiEnv is an extension of NS3Env, so that the nodes in the network can be 
completely regarded as independent agents, which have their own states, 
observations, and rewards. 

NOTE: Strictly, NS3Env is not completely distributed. Its NS-3 network has 
only one reward, and all nodes share the reward.

NOTE: We formalize the network problem as a multi-agent extension Markov decision 
processes (MDPs) called Partially Observable Markov Games (POMGs).

Why we use a multi-agent environment?

Fully-distributed learning:
Algorithms with centralized learning process are not applicable 
in the real computer network. The centralized learning controller 
is usually unable to gather collected environment transitions 
from widely distributed routers once an action is executed somewhere 
and to update the parameters of each neural network simultaneously 
caused by the limited bandwidth. [FROM: You, Xinyu, et al. 
"Toward Packet Routing with Fully-distributed Multi-agent Deep 
Reinforcement Learning." arXiv preprint arXiv:1905.03494 (2019).]

Code Reference: Lowe R, Wu Y, Tamar A, et al. Multi-agent actor-critic for 
mixed cooperative-competitive environments[C]//Advances in neural information 
processing systems. 2017: 6379-6390. 
https://github.com/openai/multiagent-particle-envs
"""

import os
import sys
import zmq

import numpy as np
import gym
from gym import spaces
from ns3gym.start_sim import  start_sim_script
import ns3gym.messages_pb2 as pb
from google.protobuf.any_pb2 import Any

class MultiZmqBridge(object):
    """
    Multi-agent NS-3 ZMQ Bridge
    """
    def __init__(self, port=0, startSim=False, simSeed=0, simArgs={}, debug=False):
        super(MultiZmqBridge, self).__init__()
        port = int(port)
        self.port = port
        self.startSim = startSim
        self.simSeed = simSeed
        self.simArgs = simArgs
        self.envStopped = False
        self.simPid = None
        self.wafPid = None
        self.ns3Process = None
        self.agents = None

        context = zmq.Context()
        self.socket = context.socket(zmq.REP)
        try:
            if port == 0 and self.startSim:
                port = self.socket.bind_to_random_port('tcp://*', min_port=5001, max_port=10000, max_tries=100)
                print("Got new port for ns3gm interface: ", port)

            elif port == 0 and not self.startSim:
                print("Cannot use port %s to bind" % str(port))
                print("Please specify correct port")
                sys.exit()

            else:
                self.socket.bind("tcp://*:%s" % str(port))

        except Exception as e:
            print("Cannot bind to tcp://*:%s as port is already in use" % str(port))
            print("Please specify different port or use 0 to get free port")
            sys.exit()

        if (startSim == True and simSeed == 0):
            maxSeed = np.iinfo(np.uint32).max
            simSeed = np.random.randint(0, maxSeed)
            self.simSeed = simSeed

        if self.startSim:
            # run simulation script
            self.ns3Process = start_sim_script(port, simSeed, simArgs, debug)
        else:
            print("Waiting for simulatison script to connect on port: tcp://localhost:{}".format(port))
            print('Please start proper ns-3 simulation script using ./waf --run "..."')

        # configure spaces
        self.agentIdVec = []
        self.observation_space = []
        self.action_space = []

        self.forceEnvStop = False

        self.obs_n = []
        self.reward_n = []
        self.done_n = []
        self.info_n = {'n': []}
        self.newEnvStateRx = None

    def close(self):
        try:
            if not self.envStopped:
                self.envStopped = True
                self.rx_env_state()
                self.send_close_command()
                self.ns3Process.kill()
                if self.simPid:
                    os.kill(self.simPid, signal.SIGTERM)
                    self.simPid = None
                if self.wafPid:
                    os.kill(self.wafPid, signal.SIGTERM)
                    self.wafPid = None
        except Exception as e:
            pass

    def _create_space(self, spaceDesc):
        space = None
        if (spaceDesc.type == pb.Discrete):
            discreteSpacePb = pb.DiscreteSpace()
            spaceDesc.space.Unpack(discreteSpacePb)
            space = spaces.Discrete(discreteSpacePb.n)

        elif (spaceDesc.type == pb.Box):
            boxSpacePb = pb.BoxSpace()
            spaceDesc.space.Unpack(boxSpacePb)
            low = boxSpacePb.low
            high = boxSpacePb.high
            shape = tuple(boxSpacePb.shape)
            mtype = boxSpacePb.dtype

            if mtype == pb.INT:
                mtype = np.int
            elif mtype == pb.UINT:
                mtype = np.uint
            elif mtype == pb.DOUBLE:
                mtype = np.float
            else:
                mtype = np.float

            space = spaces.Box(low=low, high=high, shape=shape, dtype=mtype)

        elif (spaceDesc.type == pb.Tuple):
            mySpaceList = []
            tupleSpacePb = pb.TupleSpace()
            spaceDesc.space.Unpack(tupleSpacePb)

            for pbSubSpaceDesc in tupleSpacePb.element:
                subSpace = self._create_space(pbSubSpaceDesc)
                mySpaceList.append(subSpace)

            mySpaceTuple = tuple(mySpaceList)
            space = spaces.Tuple(mySpaceTuple)

        elif (spaceDesc.type == pb.Dict):
            mySpaceDict = {}
            dictSpacePb = pb.DictSpace()
            spaceDesc.space.Unpack(dictSpacePb)

            for pbSubSpaceDesc in dictSpacePb.element:
                subSpace = self._create_space(pbSubSpaceDesc)
                mySpaceDict[pbSubSpaceDesc.name] = subSpace

            space = spaces.Dict(mySpaceDict)

        return space

    def _create_data(self, dataContainerPb):
        if (dataContainerPb.type == pb.Discrete):
            discreteContainerPb = pb.DiscreteDataContainer()
            dataContainerPb.data.Unpack(discreteContainerPb)
            data = discreteContainerPb.data
            return data

        if (dataContainerPb.type == pb.Box):
            boxContainerPb = pb.BoxDataContainer()
            dataContainerPb.data.Unpack(boxContainerPb)
            # print(boxContainerPb.shape, boxContainerPb.dtype, boxContainerPb.uintData)

            if boxContainerPb.dtype == pb.INT:
                data = boxContainerPb.intData
            elif boxContainerPb.dtype == pb.UINT:
                data = boxContainerPb.uintData
            elif boxContainerPb.dtype == pb.DOUBLE:
                data = boxContainerPb.doubleData
            else:
                data = boxContainerPb.floatData

            # TODO: reshape using shape info
            return data

        elif (dataContainerPb.type == pb.Tuple):
            tupleDataPb = pb.TupleDataContainer()
            dataContainerPb.data.Unpack(tupleDataPb)

            myDataList = []
            for pbSubData in tupleDataPb.element:
                subData = self._create_data(pbSubData)
                myDataList.append(subData)

            data = tuple(myDataList)
            return data

        elif (dataContainerPb.type == pb.Dict):
            dictDataPb = pb.DictDataContainer()
            dataContainerPb.data.Unpack(dictDataPb)

            myDataDict = {}
            for pbSubData in dictDataPb.element:
                subData = self._create_data(pbSubData)
                myDataDict[pbSubData.name] = subData

            data = myDataDict
            return data

    def _pack_data(self, actions, spaceDesc):
        dataContainer = pb.DataContainer()

        spaceType = spaceDesc.__class__

        if spaceType == spaces.Discrete:
            dataContainer.type = pb.Discrete
            discreteContainerPb = pb.DiscreteDataContainer()
            discreteContainerPb.data = actions
            dataContainer.data.Pack(discreteContainerPb)

        elif spaceType == spaces.Box:
            dataContainer.type = pb.Box
            boxContainerPb = pb.BoxDataContainer()
            shape = [len(actions)]
            boxContainerPb.shape.extend(shape)

            if (spaceDesc.dtype in ['int', 'int8', 'int16', 'int32', 'int64']):
                boxContainerPb.dtype = pb.INT
                boxContainerPb.intData.extend(actions)

            elif (spaceDesc.dtype in ['uint', 'uint8', 'uint16', 'uint32', 'uint64']):
                boxContainerPb.dtype = pb.UINT
                boxContainerPb.uintData.extend(actions)

            elif (spaceDesc.dtype in ['float', 'float32', 'float64']):
                boxContainerPb.dtype = pb.FLOAT
                boxContainerPb.floatData.extend(actions)

            elif (spaceDesc.dtype in ['double']):
                boxContainerPb.dtype = pb.DOUBLE
                boxContainerPb.doubleData.extend(actions)

            else:
                boxContainerPb.dtype = pb.FLOAT
                boxContainerPb.floatData.extend(actions)

            dataContainer.data.Pack(boxContainerPb)

        elif spaceType == spaces.Tuple:
            dataContainer.type = pb.Tuple
            tupleDataPb = pb.TupleDataContainer()

            spaceList = list(self._action_space.spaces)
            subDataList = []
            for subAction, subActSpaceType in zip(actions, spaceList):
                subData = self._pack_data(subAction, subActSpaceType)
                subDataList.append(subData)

            tupleDataPb.element.extend(subDataList)
            dataContainer.data.Pack(tupleDataPb)

        elif spaceType == spaces.Dict:
            dataContainer.type = pb.Dict
            dictDataPb = pb.DictDataContainer()

            subDataList = []
            for sName, subAction in actions.items():
                subActSpaceType = self._action_space.spaces[sName]
                subData = self._pack_data(subAction, subActSpaceType)
                subData.name = sName
                subDataList.append(subData)

            dictDataPb.element.extend(subDataList)
            dataContainer.data.Pack(dictDataPb)

        return dataContainer

    def initialize_env(self, stepInterval):
        request = self.socket.recv()
        multiAgentInitMsg = pb.MultiAgentInitMsg()
        multiAgentInitMsg.ParseFromString(request)
        self.simPid = int(multiAgentInitMsg.simProcessId)
        self.wafPid = int(multiAgentInitMsg.wafShellProcessId)

        for agentInitMsg in multiAgentInitMsg.agentInitMsg:
            agent_id = agentInitMsg.agentId
            self.agentIdVec.append(agent_id)
            ob_space = self._create_space(agentInitMsg.obsSpace)
            self.observation_space.append(ob_space)
            ac_space = self._create_space(agentInitMsg.actSpace)
            self.action_space.append(ac_space)

        reply = pb.SimInitAck()
        reply.done = True
        reply.stopSimReq = False
        replyMsg = reply.SerializeToString()
        self.socket.send(replyMsg)
        return True

    def get_observation_space(self):
        return self.observation_space

    def get_action_space(self):
        return self.action_space

    def rx_env_state(self):
        request = self.socket.recv()
        multiAgentStateMsg = pb.MultiAgentStateMsg()
        multiAgentStateMsg.ParseFromString(request)

        for agentStateMsg in multiAgentStateMsg.agentStateMsg:
            # agent_id = agentStateMsg.agentId
            obs = self._create_data(agentStateMsg.obsData)
            self.obs_n.append(obs)
            self.reward_n.append(agentStateMsg.reward)
            self.done_n.append(agentStateMsg.done)
            info = agentStateMsg.info
            if not info:
                info = {}
            self.info_n['n'].append(info)

            self.newEnvStateRx = True

    def send_close_command(self):
        reply = pb.MultiAgentActMsg()
        reply.stopSimReq = True

        replyMsg = reply.SerializeToString()
        self.socket.send(replyMsg)
        self.newEnvStateRx = False
        return True

    def send_action_n(self, action_n):
        multiAgentActMsg = pb.MultiAgentActMsg()

        for i in range(len(action_n)):
            agentAct = multiAgentActMsg.agentActMsg.add()
            actionMsg = self._pack_data(action_n[i], self.action_space[i])
            agentAct.agentId = self.agentIdVec[i]
            agentAct.actData.CopyFrom(actionMsg)

        multiAgentActMsg.stopSimReq = False
        if self.forceEnvStop:
            multiAgentActMsg.stopSimReq = True
        replyMsg = multiAgentActMsg.SerializeToString()
        self.socket.send(replyMsg)
        self.newEnvStateRx = False
        return True

    def step(self, action_n):
        # exec actions for current state
        self.send_action_n(action_n)
        # get result of above mult-agent actions
        self.rx_env_state()

    # not use
    #
    # def reset(self):
    #     # record observations for each agent
    #     obs_n = []
    #     self.agents = self.world.policy_agents
    #     for agent in self.agents:
    #         obs_n.append(self._get_obs(agent))
    #     return obs_n
    #
    # # get info used for benchmarking
    # def _get_info(self, agent):
    #     if self.info_callback is None:
    #         return {}
    #     return self.info_callback(agent, self.world)
    #
    # # get observation for a particular agent
    # def _get_obs(self, agent):
    #     if self.observation_callback is None:
    #         return np.zeros(0)
    #     return self.observation_callback(agent, self.world)
    #
    # # get dones for a particular agent
    # def _get_obs(self, agent):
    #     if self.done_callback is None:
    #         return False
    #     return self.done_callback(agent, self.world)
    #
    # # get reward for a particular agent
    # def _get_reward(self, agent):
    #     if self.reward_callback is None:
    #         return 0.0
    #     return self.reward_callback(agent, self.world)

    # set env action for a particular agent
    # def _set_action(self, action, agent, action_space, time=None):
    #     if isinstance(action_space, MultiDiscrete):
    #         act = []
    #         size = action_space.high - action_space.low + 1
    #         index = 0
    #         for s in size:
    #             act.append(action[index:(index+s)])
    #             index += s
    #         action = act
    #     else:
    #         action = [action]

    def get_done_n(self):
        return self.done_n

    def get_obs_n(self):
        return self.obs_n

    def get_reward_n(self):
        return  self.reward_n

    def get_info_n(self):
        return self.info_n


class MultiEnv(gym.Env):
    """
    Multi Agent Environment
    """
    def __init__(self, stepTime=0, port=0, startSim=True, simSeed=0, simArgs={}, debug=False):
        # set required vectorized gym env property
        self.stepTime = stepTime
        self.port = port
        self.startSim = startSim
        self.simSeed = simSeed
        self.simArgs = simArgs
        self.debug = debug

        # Filled in reset function
        self.multiZmqBridge = None
        self.action_space = []
        self.observation_space = []
        self.state = []
        self.step_beyond_done = None

        self.multiZmqBridge = MultiZmqBridge(self.port, self.startSim, self.simSeed, self.simArgs, self.debug)
        self.multiZmqBridge.initialize_env(self.stepTime)
        self.action_space = self.multiZmqBridge.get_action_space()
        self.observation_space = self.multiZmqBridge.get_observation_space()
        # get first observations
        self.multiZmqBridge.rx_env_state()
        self.envDirty = False
        # self.seed()

    def get_state_n(self):
        obs_n = self.multiZmqBridge.get_obs_n()
        reward_n = self.multiZmqBridge.get_reward_n()
        done_n = self.multiZmqBridge.get_done_n()
        info_n = self.multiZmqBridge.get_info_n()
        return (obs_n, reward_n, done_n, info_n)


    def step(self, action_n):
        self.multiZmqBridge.step(action_n)
        self.envDirty = True
        return self.get_state_n()

    def reset(self):
        if not self.envDirty:
            obs_n = self.multiZmqBridge.get_obs_n()
            return obs_n

        if self.multiZmqBridge:
            self.multiZmqBridge.close()
            self.multiZmqBridge = None

        self.envDirty = False
        self.multiZmqBridge = MultiZmqBridge(self.port, self.startSim, self.simSeed, self.simArgs, self.debug)
        self.multiZmqBridge.initialize_env(self.stepTime)
        self.action_space = self.multiZmqBridge.get_action_space()
        self.observation_space = self.multiZmqBridge.get_observation_space()
        # get first observations
        self.multiZmqBridge.rx_env_state()
        obs = self.multiZmqBridge.get_obs_n()

        return obs

    def render(self, mode='human'):
        return

    def close(self):
        if self.multiZmqBridge:
            self.multiZmqBridge.close()
            self.multiZmqBridge = None
