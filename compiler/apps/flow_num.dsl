sfun int count_flow(IP sip, IP dip, PORT sp, PORT dp) =
    filter_tcp(sip,dip,sp,dp) >>
    filter_ctrl >> 
    sum{iter([fin==1]*[syn==1]*[syn==1][fin==1]?1)};

sfun int count_flow_num =
    sum{count_flow(s,t,srp,dsp) | IP s, IP t, PORT srp, PORT dsp};




// life time
sfun int 
    filter_tcp(sip,dip,sp,dp) >>
    filter_ctrl >> 
    sum{iter([fin==1]*[syn==1]*[syn==1][fin==1]?1)};

    sum{iter(select{[fin==1]*[syn==1]*[syn==1, time==x][fin==1, time==y]?y-x})};
[syn==1, time==x][fin==1, time==y]?y-x


sfun 
    [syn==1][syn==1, ack==1][ack==1]
