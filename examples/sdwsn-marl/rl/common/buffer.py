__author__ = "Zhangmin Wang"
__email__ = "zhangmwg@gmail.com"

import random
import numpy as np


class ReplayBuffer(object):
    """
    a ring buffer for storing transitions and sampling for training
    :state: (state_dim,)
    :action: (action_dim,)
    :reward: (,), scalar
    :next_state: (state_dim,)
    :done: (,), scalar (0 and 1) or bool (True and False)
    """

    def __init__(self, capacity):
        self.capacity = capacity
        self.buffer = []
        self.position = 0  # pointer

    def push(self, state, action, reward, next_state, done):
        if len(self.buffer) < self.capacity:
            self.buffer.append(None)
        self.buffer[self.position] = (state, action, reward, next_state, done)
        self.position = int((self.position + 1) % self.capacity)  # as a ring buffer

    def sample(self, bstch_size):
        batch = random.sample(self.buffer, bstch_size)
        state, action, reward, next_state, done = map(np.stack, zip(*batch))  # stack for each element
        return state, action, reward, next_state, done

    def __len__(self):
        return len(self.buffer)
