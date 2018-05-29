#!/usr/bin/python

from time import sleep
from time import time
from signal import SIGINT
from functools import partial
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Controller
from mininet.node import RemoteController
from mininet.node import OVSController
from mininet.node import CPULimitedHost
from mininet.node import OVSSwitch
from mininet.link import TCLink
from mininet.util import dumpNodeConnections
from mininet.util import pmonitor
from mininet.log import setLogLevel
from mininet.cli import CLI
import datetime


logFile = 'mininet.log'

class DefaultController(Controller):
  def start(self):
    self.cmd('./pox/pox.py openflow.of_01 --port=6633 forwarding.l2_forwarding &')

  def stop(self):
    self.cmd('kill %./pox/pox.py')


def runTest():
  f = open(logFile, 'wb')

  numHosts = 4
  net = Mininet(switch=OVSSwitch, link=TCLink, build=False)

  print "*** Creating (reference) controllers"
  #c1 = DefaultController('c1', port=6633)
  #c1 = RemoteController('c1', port=6633)
  c1 = OVSController('c1', port=6633)
  net.addController(c1)

  print "*** Creating switches"
  s1 = net.addSwitch('s1', dpopts='')

  print "*** Creating hosts"
  hosts = [net.addHost('h%d'%n, cpu=1/numHosts) for n in range(1, numHosts+1)]

  print "*** Creating links"
  linkopts = dict(bw=100)
  for h in hosts:
    net.addLink(h, s1, **linkopts)
  
  print "*** Starting network"
  net.build()
  c1.start()
  s1.start([c1])

  hosts = net.hosts

  client1 = hosts[0]
  client2 = hosts[1]
  server = hosts[2]
  recon = hosts[3]

  server.cmd('iperf -s -p 5001 &')
  #client1.sendCmd('iperf -c %s -p 5001 -i 1 -t 600 -b 1m &' % str(server.IP()))

  '''
  beginTime = datetime.datetime.now()
  popens = {}
  popens[client1] = client1.popen('iperf -c %s -p 5001 -i 1 -t 600 -b 1m' % str(server.IP()))
  for h, line in pmonitor(popens):
    delta = datetime.datetime.now() - beginTime
    time = delta.seconds + delta.microseconds/1E6
    if h is not None:
      log('{0} {1} {2}'.format(h.name, time, line))
    if time > 5:
      break

  popens[client2] = client2.popen('iperf -c %s -p 5001 -i 1 -t 600 -b 10m' % str(server.IP()))
  for h, line in pmonitor(popens):
    delta = datetime.datetime.now() - beginTime
    time = delta.seconds + delta.microseconds/1E6

    if h is not None:
      log('{0} {1} {2}'.format(h.name, time, line))
  '''

  CLI(net)
  #net.stop()


def log(line):
  print line
  #f.write(line)


if __name__ == '__main__':
  setLogLevel('info')
  runTest()



