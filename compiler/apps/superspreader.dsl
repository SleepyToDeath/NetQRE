// count total number of distinct IP addresses for each source
sfun int f(ip x) = 
    sum{.*[src==x, dst==y].*?1 | ip y} ;
