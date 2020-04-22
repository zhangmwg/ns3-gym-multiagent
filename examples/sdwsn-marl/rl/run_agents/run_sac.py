__author__ = "Zhangmin Wang"
__email__ = "zhangmwg@gmail.com"

import gym
import numpy as np

from algorithms.sac import SAC
from common.value_networks import *
from common.policy_networks import *
import tensorflow as tf
from ns3gym import ns3env

''' load environment '''
# env = gym.make('Pendulum-v0').unwrapped
# env = DummyVecEnv([lambda: env])  # The algorithms require a vectorized/wrapped environment to run
env = ns3env.Ns3Env()
action_shape = env.action_space.shape
state_shape = env.observation_space.shape
# reproducible
seed = 2
np.random.seed(seed)
tf.random.set_seed(seed)
env.seed(seed)

''' build networks for the algorithm '''
num_hidden_layer = 4  # number of hidden layers for the networks
hidden_dim = 64  # dimension of hidden layers for the networks, default as the same for each layer here
with tf.name_scope('SAC'):
    with tf.name_scope('Q_Net1'):
        soft_q_net1 = MlpQNetwork(state_shape, action_shape, hidden_dim_list=num_hidden_layer * [hidden_dim])
    with tf.name_scope('Q_Net2'):
        soft_q_net2 = MlpQNetwork(state_shape, action_shape, hidden_dim_list=num_hidden_layer * [hidden_dim])
    with tf.name_scope('Target_Q_Net1'):
        target_soft_q_net1 = MlpQNetwork(state_shape, action_shape, hidden_dim_list=num_hidden_layer * [hidden_dim])
    with tf.name_scope('Target_Q_Net2'):
        target_soft_q_net2 = MlpQNetwork(state_shape, action_shape, hidden_dim_list=num_hidden_layer * [hidden_dim])
    with tf.name_scope('Policy'):
        policy_net = StochasticPolicyNetwork(env.observation_space, env.action_space,
                                             hidden_dim_list=num_hidden_layer * [hidden_dim])
net_list = [soft_q_net1, soft_q_net2, target_soft_q_net1, target_soft_q_net2, policy_net]

''' choose optimizers '''
soft_q_lr, policy_lr, alpha_lr = 3e-4, 3e-4, 3e-4  # soft_q_lr: learning rate of the Q network; policy_lr: learning rate of the policy network; alpha_lr: learning rate of the variable alpha
soft_q_optimizer1 = tf.optimizers.Adam(soft_q_lr)
soft_q_optimizer2 = tf.optimizers.Adam(soft_q_lr)
policy_optimizer = tf.optimizers.Adam(policy_lr)
alpha_optimizer = tf.optimizers.Adam(alpha_lr)
optimizers_list = [soft_q_optimizer1, soft_q_optimizer2, policy_optimizer, alpha_optimizer]

model = SAC(net_list, optimizers_list, state_dim=state_shape[0], action_dim=action_shape[0])
''' 
full list of arguments for the algorithm
----------------------------------------
net_list: a list of networks (value and policy) used in the algorithm, from common functions or customization
optimizers_list: a list of optimizers for all networks and differentiable variables
state_dim: dimension of state for the environment
action_dim: dimension of action for the environment
replay_buffer_capacity: the size of buffer for storing explored samples
action_range: value of each action in [-action_range, action_range]
'''

model.learn(env, train_episodes=100, max_steps=150, batch_size=64, explore_steps=500, \
            update_itr=3, policy_target_update_interval=3, reward_scale=1., save_interval=20, \
            mode='train', AUTO_ENTROPY=True, DETERMINISTIC=False, render=False)
''' 
full list of parameters for training
---------------------------------------
env: learning environment
train_episodes:  total number of episodes for training
test_episodes:  total number of episodes for testing
max_steps:  maximum number of steps for one episode
batch_size:  udpate batchsize
explore_steps:  for random action sampling in the beginning of training
update_itr: repeated updates for single step
policy_target_update_interval: delayed update for the policy network and target networks
reward_scale: value range of reward
save_interval: timesteps for saving the weights and plotting the results
mode: 'train'  or 'test'
AUTO_ENTROPY: automatically udpating variable alpha for entropy
DETERMINISTIC: stochastic action policy if False, otherwise deterministic
render: if true, visualize the environment
'''
# test
model.learn(env, test_episodes=10, max_steps=150, mode='test', render=False)
