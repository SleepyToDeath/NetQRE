- use external compiler to generate executable program
	- modify program to directly generate executable program from source code
	- modify syntax tree to generate source code

- generate grammar from json
	- wrapper for json library
	- grammar generator ([TODO]scan name)

- test 
	- write grammar
	- debug & test
	- make some modification
	- debug & test

- train templates
	- template: a (incomplete) program a LHS can mutate into
		e.g. [re -> star char star char star char star] (can be useful in matching ipv4)
	- find a way to measure better template overall during training (more templates may even slow down the search)
	- find a way to measure probability of an LHS during search


