#!/usr/bin/python

from time import sleep
import datetime
from signal import SIGINT
from functools import partial

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Controller
from mininet.node import RemoteController
from mininet.node import CPULimitedHost
from mininet.link import TCLink
from mininet.util import dumpNodeConnections
from mininet.util import pmonitor
from mininet.log import setLogLevel
from mininet.cli import CLI

logFile = 'mininet.log'

add_mirror_cmd = """ovs-vsctl -- set bridge "s1" mirrors=@m \\
-- --id=@s1-eth1 get port s1-eth1 \\
-- --id=@s1-eth2 get port s1-eth2 \\
-- --id=@s1-eth3 get port s1-eth3 \\
-- --id=@s1-eth4 get port s1-eth4 \\
-- --id=@m create mirror name=m0 select-dst-port=@s1-eth1,@s1-eth2,@s1-eth3 select-src-port=@s1-eth1,@s1-eth2,@s1-eth3 output-port=@s1-eth4
"""

class SingleSwitchTopo(Topo):
  def __init__(self, numHosts):
    Topo.__init__(self)
    linkopts = dict(bw=100)
    switch = self.addSwitch('s1', dpopts='')
    hosts = {}

    for i in range(numHosts):
      host = self.addHost('h%s' % (i+1), cpu=0.5/numHosts, mac='00:00:00:00:00:0%s' % (i+1))
      hosts['h%s' % (i+1)] = host
      self.addLink(host, switch, **linkopts)

def runTest():
  f = open(logFile, 'wb')

  topo = SingleSwitchTopo(4)
  net = Mininet(topo, controller=RemoteController, link=TCLink)

  net.addNAT().configDefault()
  net.start()

  popens = {}
  hosts = net.hosts

  switch = net['s1']
  client1 = hosts[0]
  client2 = hosts[1]
  server = hosts[2]
  recon = hosts[3]

  switch.cmd(add_mirror_cmd)
  server.cmd('iperf -s -p 5001 &')
  server.cmd('./sipp/sipp -sn uas -mi %s &' % str(server.IP()))
  server.cmd('./bandwidth_recorder live h3-eth0 &')
  sleep(2) # wait for server to listen to its port before recon connects to that port
  recon.cmd('./sip live h4-eth0 &')
  #client1.sendCmd('iperf -c %s -p 5001 -i 1 -t 600 -b 1m &' % str(server.IP()))
  #client2.sendCmd('iperf -c %s -p 5001 -i 1 -t 600 -b 10m &' % str(server.IP()))

  beginTime = datetime.datetime.now()

  popens[client1] = client1.popen('iperf -c %s -p 5001 -i 1 -t 600 -b 1m' % str(server.IP()))
  for h, line in pmonitor(popens):
    delta = datetime.datetime.now() - beginTime
    time = delta.seconds + delta.microseconds/1E6
    if h is not None:
      log('{0} {1} {2}'.format(h.name, time, line))
    if time > 5:
      break

  popens[client2] = client2.popen('./sipp/sipp -sn uac_pcap %s -mi %s -l 1' % (str(server.IP()), str(client2.IP())))
  '''
  for h, line in pmonitor(popens):
    delta = datetime.datetime.now() - beginTime
    time = delta.seconds + delta.microseconds/1E6

    if h is not None:
      log('{0} {1} {2}'.format(h.name, time, line))
  '''

  CLI(net)
  #net.stop()

def log(line):
  pass
  #print line
  #f.write(line)


if __name__ == '__main__':
  setLogLevel('info')
  runTest()



