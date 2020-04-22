__author__ = "Zhangmin Wang"
__email__ = "zhangmwg@gmail.com"

""" https://github.com/PKU-AI-Edge/DGN/blob/master/Routing/routers.py """

import tensorflow as tf
import numpy as np

Input = tf.keras.layers.Input
Dense = tf.keras.layers.Dense
Lambda = tf.keras.layers.Lambda
Reshape = tf.keras.layers.Reshape
K = tf.keras.backend
Model = tf.keras.models.Model


def MultiHeadsAttModel(l=2, d=128, dv=16, dout=128, nv = 8 ):

	v1 = Input(shape = (l, d))
	q1 = Input(shape = (l, d))
	k1 = Input(shape = (l, d))
	ve = Input(shape = (1, l))

	v2 = Dense(dv*nv, activation = "relu",kernel_initializer='random_normal')(v1)
	q2 = Dense(dv*nv, activation = "relu",kernel_initializer='random_normal')(q1)
	k2 = Dense(dv*nv, activation = "relu",kernel_initializer='random_normal')(k1)

	v = Reshape((l, nv, dv))(v2)
	q = Reshape((l, nv, dv))(q2)
	k = Reshape((l, nv, dv))(k2)
	v = Lambda(lambda x: K.permute_dimensions(x, (0,2,1,3)))(v)
	k = Lambda(lambda x: K.permute_dimensions(x, (0,2,1,3)))(k)
	q = Lambda(lambda x: K.permute_dimensions(x, (0,2,1,3)))(q)

	att = Lambda(lambda x: K.batch_dot(x[0],x[1] ,axes=[3,3]) / np.sqrt(dv))([q,k])# l, nv, nv
	att = Lambda(lambda x: K.softmax(x))(att)
	out = Lambda(lambda x: K.batch_dot(x[0], x[1],axes=[3,2]))([att, v])
	out = Lambda(lambda x: K.permute_dimensions(x, (0,2,1,3)))(out)

	out = Reshape((l, dv*nv))(out)

	T = Lambda(lambda x: K.batch_dot(x[0],x[1]))([ve,out])

	out = Dense(dout, activation = "relu",kernel_initializer='random_normal')(T)
	model = Model(inputs=[q1,k1,v1,ve], outputs=out)
	return model

# m1 = MultiHeadsAttModel(l=neighbors)