// count size of traffic from x to y
sfun int hh(IP x, IP y) = 
    sum{iter([srcip==x, dstip==y]?size)};



