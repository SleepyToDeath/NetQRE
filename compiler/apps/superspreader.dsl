// count total number of distinct IP addresses for each source
sfun int f = 
    sum{/.*[src==x].*/?1 | IP x};

