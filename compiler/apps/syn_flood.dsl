// syn packets without acks
sfun int syn_flood(IP x) = 
    sum{iter([syn==1][syn==0]*?1)};

