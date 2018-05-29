// test code to check variable scoping
sfun int f() = 
	sum{ /[src==x]/ | ip x} + sum{ /[dst==x]/ | ip x};
