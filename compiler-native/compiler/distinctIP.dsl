// count how many distinct IP addr so far.
sfun int count_ip(int x) = 
    sum{iter([src!=x]*[src==x] ? 1)};
