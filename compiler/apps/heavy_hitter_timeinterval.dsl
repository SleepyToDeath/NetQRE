// count size of traffic from x to y in recent 5 seconds
fun packet cut(packet pkt) = {
    int time = time/1000;
    last
}
    
sfun int hh(IP x, IP y) = 
//    [time==1]? cut(last)
//>>
    sum{iter(
	[srcip==x, dstip==y, time>(time-5)]?size
	)
    };
