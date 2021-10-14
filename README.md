# ns3-gym for multi-agent

MultiEnv is an extension of [ns3-gym](https://github.com/tkn-tub/ns3-gym), so that the nodes in the network can be completely regarded as independent agents, which have their own states, observations, and rewards. 

NOTE: We formalize the network problem as a multi-agent extension Markov decision processes (MDPs) called Partially Observable Markov Games (POMGs).

**Why we use a multi-agent environment?**

**Fully-distributed learning:** Algorithms with centralized learning process are not applicable in the real computer network. The centralized learning controller is usually unable to gather collected environment transitions from widely distributed routers once an action is executed somewhere and to update the parameters of each neural network simultaneously caused by the limited bandwidth. [FROM: You, Xinyu, et al. "Toward Packet Routing with Fully-distributed Multi-agent Deep Reinforcement Learning." arXiv preprint arXiv:1905.03494 (2019).]

Code Reference: Lowe R, Wu Y, Tamar A, et al. Multi-agent actor-critic for mixed cooperative-competitive environments[C]//Advances in neural information processing systems. 2017: 6379-6390. https://github.com/openai/multiagent-particle-envs

## How (Python)
```python
from ns3gym import ns3_multiagent_env as ns3env
```

## How (ns-3)
```C++
///\{ Each agent OpenGym Env 
  virtual Ptr<OpenGymSpace> GetActionSpace(uint32_t agent_id) = 0;
  virtual Ptr<OpenGymSpace> GetObservationSpace(uint32_t agent_id) = 0;
  virtual Ptr<OpenGymDataContainer> GetObservation(uint32_t agent_id) = 0;
  virtual float GetReward(uint32_t agent_id) = 0;
  virtual bool GetDone(uint32_t agent_id) = 0;
  virtual std::string GetInfo(uint32_t agent_id) = 0;
  virtual bool ExecuteActions(uint32_t agent_id, Ptr<OpenGymDataContainer> action) = 0;
  ///\}
```
## Example 
### multi-agent example 1
This example shows how to create an ns3-gym environment with multiple agents in one Python processes. Similar to the 
[multiagent-particle-envs](https://github.com/openai/multiagent-particle-envs)

[multi-agent example 1](https://github.com/zhangmwg/ns3-gym-multiagent/tree/master/examples/multigym)

### multi-agent example 2
This example shows how to create an ns3-gym environment with multiple agents and connects them to multiple independent Python processes.
[multi-agent example 2](https://github.com/zhangmwg/ns3-gym-multiagent/tree/master/examples/multi-agent)

ns3-gym
============

[OpenAI Gym](https://gym.openai.com/) is a toolkit for reinforcement learning (RL) widely used in research. The network simulator [ns-3](https://www.nsnam.org/) is the de-facto standard for academic and industry studies in the areas of networking protocols and communication technologies. [ns3-gym](https://github.com/tkn-tub/ns3-gym) is a framework that integrates both OpenAI Gym and ns-3 in order to encourage usage of RL in networking research.

Installation
============

https://github.com/tkn-tub/ns3-gym

How to reference ns3-gym?
============

Please use the following bibtex :

```
@inproceedings{ns3gym,
  Title = {{ns-3 meets OpenAI Gym: The Playground for Machine Learning in Networking Research}},
  Author = {Gaw{\l}owicz, Piotr and Zubow, Anatolij},
  Booktitle = {{ACM International Conference on Modeling, Analysis and Simulation of Wireless and Mobile Systems (MSWiM)}},
  Year = {2019},
  Location = {Miami Beach, USA},
  Month = {November},
  Url = {http://www.tkn.tu-berlin.de/fileadmin/fg112/Papers/2019/gawlowicz19_mswim.pdf}
}
```

```
@article{ns3gym,
  author    = {Gawlowicz, Piotr and Zubow, Anatolij},
  title     = {{ns3-gym: Extending OpenAI Gym for Networking Research}},
  journal   = {CoRR},
  year      = {2018},
  url       = {https://arxiv.org/abs/1810.03943},
  archivePrefix = {arXiv},
}
```
