// This is a simple state firewall, which allows packets from port 1,
// and for packet coming from port 2, it checks whether
// there is a packet in the history with src == its dstip, 
// and coming from port 1
sfun action sf = {
    if (last.inport==1) 
	allow;
    else {
	.*[inport==1, srcip==last.dstip].* ? allow:drop
	//.*[srcip==last.dstip].* ? allow:drop
    }
}
