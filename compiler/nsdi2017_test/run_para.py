import sys
import os

ddostrace = "/home/yifei/ddostrace.20070804_142436.pcap"
normaltrace = "/home/yifei/equinix-chicago.dirB.20150219-135900.UTC.anon.pcap"

filename = sys.argv[1]
num_run = int(sys.argv[2])

f = open(filename+".log",'w')

for num_thread in range(10, 2, -1):
    f.write("Run {} on {} with {} threads {} times.\n".format(filename, ddostrace, num_thread, num_run))

    for i in range(0,num_run):
	f.write(os.popen("./{} offline {} 1 0 {}".format(filename, ddostrace, num_thread)).read())


    f.write("\n\nRun {} on {} with {} threads {} times.\n".format(filename, normaltrace, num_thread, num_run))

    for i in range(0,num_run):
	f.write(os.popen("./{} offline {} 1 0 {}".format(filename, normaltrace, num_thread)).read())
