__author__ = "Zhangmin Wang"
__email__ = "zhangmwg@gmail.com"

import operator
import os
import re
import matplotlib.pyplot as plt
from importlib import import_module
import numpy as np

import tensorlayer as tl
import tensorflow as tf


def plot_save_log(episode_rewards, Algorithm_name, Env_name):
    '''
    plot the learning curve, saved as ./img/Algorithm_name-Env_name.png,
    and save the rewards log as ./log/Algorithm_name-Env_name.npy
    :episode_rewards: array of floats
    :Algorithm_name: string
    :Env_name: string
    '''
    plt.figure(figsize=(10, 5))
    plt.title(Algorithm_name + '-' + Env_name)
    plt.plot(np.arange(len(episode_rewards)), episode_rewards)
    plt.xlabel('Episode')
    plt.ylabel('Episode Reward')
    if not os.path.exists('img'):
        os.makedirs('img')
    plt.savefig('./img/' + Algorithm_name + '-' + Env_name + '.png')
    if not os.path.exists('log'):
        os.makedirs('log')
    np.save('./log/' + Algorithm_name + '-' + Env_name, episode_rewards)
    plt.close()


def save_model(model, Model_name, Algorithm_name):
    '''
    save trained neural network model
    :model: tensorlayer.models.Model
    :Model_name: string, e.g. 'model_sac_q1'
    :Algorithm_name: string, e.g. 'SAC'
    '''
    if not os.path.exists('model/' + Algorithm_name):
        os.makedirs('model/' + Algorithm_name)
    tl.files.save_npz(model.trainable_weights, './model/' + Algorithm_name + '/' + Model_name)


def load_model(model, Model_name, Algorithm_name):
    '''
    load saved neural network model
    :model: tensorlayer.models.Model
    :Model_name: string, e.g. 'model_sac_q1'
    :Algorithm_name: string, e.g. 'SAC'
    '''
    try:
        tl.files.load_and_assign_npz('./model/' + Algorithm_name + '/' + Model_name + '.npz', model)
    except:
        print('Load Model Fails!')
