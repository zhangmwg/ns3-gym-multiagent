#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
from ns3gym import ns3_multiagent_env as ns3env

__author__ = "ZhangminWang"
__copyright__ = "Copyright (c) 2019"
__version__ = "0.1.1"

# NOTE: This is only a test, not use algorithm

parser = argparse.ArgumentParser(description='Start simulation script on/off')
parser.add_argument('--start',
                    type=int,
                    default=1,
                    help='Start ns-3 simulation script 0/1, Default: 1')
parser.add_argument('--iterations',
                    type=int,
                    default=10,
                    help='Number of iterations, Default: 10')
args = parser.parse_args()
startSim = True
iterationNum = int(args.iterations)

port = 5555
simTime = 5 # seconds
stepTime = 0.5  # seconds
seed = 0
simArgs = {"--simTime": simTime,
           "--stepTime": stepTime,
           "--testArg": 123}
debug = False

env = ns3env.MultiEnv(port=port, stepTime=stepTime, startSim=startSim, simSeed=seed, simArgs=simArgs, debug=debug)
# simpler:
#env = ns3env.Ns3Env()
env.reset()
ob_spaces = env.observation_space
ac_spaces = env.action_space

print("Observation space: ", ob_spaces, " size: ", len(ob_spaces))
print("Action space: ", ac_spaces, " size: ", len(ac_spaces))

stepIdx = 0
currIt = 0
agent_num = len(ob_spaces)

try:
    while True:
        print("Start iteration: ", currIt)
        obs = env.reset()
        print("---obs: ", obs)

        while True:
            stepIdx += 1
            print("Step: ", stepIdx)
            actions = []
            for ag in range(agent_num):
                action = env.action_space[ag].sample()
                print("---action: ", action)
                actions.append(action)

            obs, reward, _, _ = env.step(actions)
            print("---obs, reward: ", obs, reward)

            if stepIdx == 10:
                stepIdx = 0
                break

        currIt += 1
        if currIt == iterationNum:
            break

except KeyboardInterrupt:
    print("Ctrl-C -> Exit")
finally:
    env.close()
    print("Done")