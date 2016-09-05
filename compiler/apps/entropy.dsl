sfun int total = sum{iter(/./?1)};

sfun int count_src(IP x) = sum{iter(/[srcip==x]/?1)};

sfun int frequency(IP x) = count_src(x)/total;

sfun int entropy = sum{frequency(x)*log(frequency(x)) | IP x};


//sfun double entropy = 
//    sum{
//	(sum{iter([srcip==x]?1)}
//	    /sum{iter(.?1)})
//	 *
//	log(sum{iter([srcip==x]?1)}
//	    /sum{iter(.?1)})
//	| IP x
//    };
