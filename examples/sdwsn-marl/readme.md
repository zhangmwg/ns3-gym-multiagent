# Routing Optimization Using Multi-Agent Reinforcement Learning in Software-Defined Wireless Sensor Networks

Autohor: <font color=Blue> Zhangmin Wang </font> 

Email: zhangmwg@gmail.com

#### <font color=blue>WSN Routing Example</font>
ns-3.29/examples/wsn-routing/
## Run
ns-3.29$ ./waf --run examples/wsn-routing/wsn-routing-example
## Debug
ns-3.29$ ./waf --run examples/wsn-routing/wsn-routing-example --command-template="gdb %s"
## 输出结果位置
ns-3.29/
## build
ns-3.29$./waf configure --build-profile=debug --enable-examples --enable-tests
### Python biliding <font color=red>python3 (ubuntu 18)</font>
<font color=blue>ns-3 绑定特定版本的 python (ubuntu 18 系统下的 python 3.6)</font>

ns-3.29$ /usr/bin/python3 ./waf configure --build-profile=debug --enable-examples --enable-tests

###<font color=red>[bug]</font>
<font color=red>When using python3 waf in ns-3.29, the visualizer module has a w_char** bug that requires a simple fix. It is also possible not to compile the visualizer module.</font>

##### Note: Please put sdwsn-romarl in opengym/examples/
#### Requirement: 
#####ns-3 (3.29 or later, we use ns-3.29), gym (0.12 or later), ns3-gym (rename: opengym), TensorFlow_v2, TensorLayer, python3 (we use py3.6), Ubuntu (16 or later, we use Ubuntu 18.04)

ns-3: https://www.nsnam.org/, ns3-gym: https://apps.nsnam.org/app/ns3-gym/, gym: http://gym.openai.com/

#####sdwsn-romarl/rl: Reinforcement Learning Algorithms

#####sdwsn-romarl/gym: Environment for SD-WSN