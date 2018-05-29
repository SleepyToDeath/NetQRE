//sfun string get_data(ip x) = 
   //[srcip==x]?last.data;
sfun int count = 
    //get_data(1) >> 
   [srcip==x]?last.data
>>
    sum{iter([C==1][C==0]?1)};
