__author__ = "Zhangmin Wang"
__email__ = "zhangmwg@gmail.com"

import numpy as np
import tensorflow as tf
from gym import spaces
import copy
from functools import wraps

def expand_dims(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        result = func(*args, **kwargs)
        result = tf.expand_dims(result, axis=-1)
        return result

    return wrapper

class Distribution(object):
    """A particular probability distribution"""

    def set_param(self, *args, **kwargs):
        raise NotImplementedError

    def sample(self, *args, **kwargs):
        """Sampling from distribution. Allow explore parameters."""
        raise NotImplementedError

    def logp(self, x):
        """Calculate log probability of a sample."""
        return -self.neglogp(x)

    def neglogp(self, x):
        """Calculate negative log probability of a sample."""
        raise NotImplementedError

    def kl(self, *parameters):
        """Calculate Kullback–Leibler divergence"""
        raise NotImplementedError

    def entropy(self):
        """Calculate the entropy of distribution."""
        raise NotImplementedError


class Categorical(Distribution):
    """Creates a categorical distribution"""

    def __init__(self, ndim, logits=None):
        """
        Args:
            ndim (int): total number of actions
            logits (tensor): logits variables
        """
        self._ndim = ndim
        self._logits = logits

    @property
    def ndim(self):
        return copy.copy(self._ndim)

    def set_param(self, logits):
        """
        Args:
            logits (tensor): logits variables to set
        """
        self._logits = logits

    def get_param(self):
        return copy.deepcopy(self._logits)

    def sample(self):
        """ Sample actions from distribution, using the Gumbel-Softmax trick """
        u = np.array(np.random.uniform(0, 1, size=np.shape(self._logits)), dtype=np.float32)
        res = tf.argmax(self._logits - tf.math.log(-tf.math.log(u)), axis=-1)
        return res

    def greedy_sample(self):
        """ Get actions greedily """
        _probs = tf.nn.softmax(self._logits)
        return tf.argmax(_probs, axis=-1)

    def logp(self, x):
        return -self.neglogp(x)

    @expand_dims
    def neglogp(self, x):
        x = np.array(x)
        if np.any(x % 1):
            raise ValueError('Input float actions in discrete action space')
        x = tf.convert_to_tensor(x, tf.int32)
        x = tf.one_hot(x, self._ndim, axis=-1)
        return tf.nn.softmax_cross_entropy_with_logits(x, self._logits)

    @expand_dims
    def kl(self, logits):
        """
        Args:
            logits (tensor): logits variables of another distribution
        """
        a0 = self._logits - tf.reduce_max(self._logits, axis=-1, keepdims=True)
        a1 = logits - tf.reduce_max(logits, axis=-1, keepdims=True)
        ea0 = tf.exp(a0)
        ea1 = tf.exp(a1)
        z0 = tf.reduce_sum(ea0, axis=-1, keepdims=True)
        z1 = tf.reduce_sum(ea1, axis=-1, keepdims=True)
        p0 = ea0 / z0
        return tf.reduce_sum(
            p0 * (a0 - tf.math.log(z0) - a1 + tf.math.log(z1)), axis=-1)

    @expand_dims
    def entropy(self):
        a0 = self._logits - tf.reduce_max(self._logits, axis=-1, keepdims=True)
        ea0 = tf.exp(a0)
        z0 = tf.reduce_sum(ea0, axis=-1, keepdims=True)
        p0 = ea0 / z0
        return tf.reduce_sum(p0 * (tf.math.log(z0) - a0), axis=-1)


def make_dist(ac_space):
    """Get distribution based on action space
    Args:
        ac_space (gym.spaces.Space)
    """
    if isinstance(ac_space, spaces.Discrete):
        return Categorical(ac_space.n)
    else:
        raise NotImplementedError