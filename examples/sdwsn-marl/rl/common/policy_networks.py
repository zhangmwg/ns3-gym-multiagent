__author__ = "Zhangmin Wang"
__email__ = "zhangmwg@gmail.com"

import operator
import os
import random
import copy

import numpy as np

import tensorflow as tf
import tensorflow_probability as tfp
import tensorlayer as tl
from tensorlayer.layers import Dense, Input
from tensorlayer.models import Model
from common.basic_nets import *
from common.distributions import make_dist
from gym import spaces

tfd = tfp.distributions
Normal = tfd.Normal


class StochasticPolicyNetwork(Model):
    def __init__(self, state_space, action_space, hidden_dim_list, w_init=tf.keras.initializers.glorot_normal(),
                 activation=tf.nn.relu, output_activation=tf.nn.tanh, log_std_min=-20, log_std_max=2, trainable=True,
                 name=None):
        """ Stochastic continuous/discrete policy network with multiple fully-connected layers

        Args:
            state_space (gym.spaces): space of the state from gym environments
            action_space (gym.spaces): space of the action from gym environments
            hidden_dim_list (list[int]): a list of dimensions of hidden layers
            w_init (callable): weights initialization
            activation (callable): activation function
            output_activation (callable or None): output activation function
            log_std_min (float): lower bound of standard deviation of action
            log_std_max (float): upper bound of standard deviation of action
            trainable (bool): set training and evaluation mode
        Tips:
            We recommend to use tf.nn.tanh for output_activation, especially for continuous action space,
            to ensure the final action range is exactly the same as declared in action space after action normalization.

        """
        self._state_space, self._action_space = state_space, action_space

        if isinstance(self._action_space, spaces.Box):  # normalize action
            assert all(self._action_space.low < self._action_space.high)
            action_bounds = [self._action_space.low, self._action_space.high]
            self._action_mean = np.mean(action_bounds, 0)
            self._action_scale = action_bounds[1] - self._action_mean

            self.policy_dist = make_dist(self._action_space)  # create action distribution
            self.policy_dist.action_mean = self._action_mean
            self.policy_dist.action_scale = self._action_scale

        else:
            self.policy_dist = make_dist(self._action_space)  # create action distribution

        self._state_shape = state_space.shape

        # build structure
        if len(self._state_shape) == 1:
            l = inputs = Input((None,) + self._state_shape, name='input_layer')
        else:
            with tf.name_scope('CNN'):
                inputs, l = CNN(self._state_shape, conv_kwargs=None)

        with tf.name_scope('MLP'):
            for i, dim in enumerate(hidden_dim_list):
                l = Dense(n_units=dim, act=activation, W_init=w_init, name='hidden_layer%d' % (i + 1))(l)

        with tf.name_scope('Output'):
            if isinstance(action_space, spaces.Discrete):
                outputs = Dense(n_units=self.policy_dist.ndim, act=output_activation, W_init=w_init)(l)
            elif isinstance(action_space, spaces.Box):
                mu = Dense(n_units=self.policy_dist.ndim, act=output_activation, W_init=w_init)(l)
                log_sigma = Dense(n_units=self.policy_dist.ndim, act=None, W_init=w_init)(l)
                log_sigma = tl.layers.Lambda(lambda x: tf.clip_by_value(x, log_std_min, log_std_max))(log_sigma)
                outputs = [mu, log_sigma]
            else:
                raise NotImplementedError

        # make model
        super().__init__(inputs=inputs, outputs=outputs, name=name)
        if trainable:
            self.train()
        else:
            self.eval()

    def __call__(self, states, *args, greedy=False, **kwargs):
        if np.shape(states)[1:] != self.state_shape:
            raise ValueError(
                'Input state shape error. Shape should be {} but your shape is {}'.format((None,) + self.state_shape,
                                                                                          np.shape(states)))
        states = np.array(states, dtype=np.float32)
        params = super().__call__(states, *args, **kwargs)
        self.policy_dist.set_param(params)
        if greedy:
            result = self.policy_dist.greedy_sample()
        else:
            result = self.policy_dist.sample()

        if isinstance(self._action_space, spaces.Box):  # normalize action
            if greedy:
                result = result * self._action_scale + self._action_mean
            else:
                result, explore = result
                result = result * self._action_scale + self._action_mean + explore

            result = tf.clip_by_value(result, self._action_space.low, self._action_space.high)
        return result

    @property
    def state_space(self):
        return copy.deepcopy(self._state_space)

    @property
    def action_space(self):
        return copy.deepcopy(self._action_space)

    @property
    def state_shape(self):
        return copy.deepcopy(self._state_shape)

    @property
    def action_shape(self):
        return copy.deepcopy(self._action_shape)