# What's this?
This is a synthesizer for NetQRE programs. It takes labeled network traces as inputs

# How to use?

##Overview
The synthesizer has 2 parts: the enumerator and the execution engine. Their functions are
straightforward from the name. There will be an enumerator process and a number of 
execution servers. Whenever enumerator want to check a program, it will be sent to the 
least busy server for execution via RPC.

This directory contains only the code for enumerator. The execution server is located
in `../netqre2dt/` and `../data-transducer/`. Its build directory is `../netqre2dt/rpc`.

##Dependency
- Any g++ version that supports C++11 (C++14 is even better)
- rpclib (may need manual installation; see `rpclib.net` for more detail; type `sudo make install` after the compile step)
- libpcap, libpcap-dev

##Compile
- `make` here
- `make` in `../netqre2dt/rpc`

##Run
You need quite some configuration files for running. Refer to `test` entry in `Makefile` for
details. 

If you only want to use the default NetQRE, then just do 3 things:
- replace `./testbed/positive.ts` and `./testbed/negative.ts` with your own data
- replace urls in 3 bash files under `../netqre2dt/rpc/` with your own machines'
- adjust the number of server instances in `../netqre2dt/rpc/Makefile` and `./lang/netqre/netqre_server_list.txt`

`ts` is short for `token stream`. They are actually plain text. Refer to `./testbed/pcap/` 
for examples of tokenizing pcap files.

A detailed document for all configurations is coming soon. If you really wonders, then
`./lang/general/main.cc` is the source code that handles all these inputs, which is not very
difficult to read.

When config and data are ready, go to `../netqre2dt/rpc`:   
`distribute.sh`    
`./runall.sh`    
Then in this directory:    
`make test`   
Then you can wait for the result.   

If your task didn't end normally, e.g., was killed, then you may want to refresh servers by:
`./killall.sh`   
`./runall.sh`   
in the server's directory before running a new task.

Whenever you change the test data, run `distribute.sh` again and then
refresh servers.


# How does it work?

Details of NetQRE can be found in our paper. Here I only introduce the intuition with regular
expression.

## The naive way
A naive way is to enumerate all programs in increasing order of size, and test every program
if it can accept all examples. Beware of the way we do the enumeration. We start from starting
symbol, repetitively mutate it following syntax rules. If the resulting program contains some 
non-terminal symbols, then it's incomplete, and we have to go on with the mutation, until it 
contains only terminals, when we can eventually test it on the examples. The search of programs 
will form a tree, where starting symbol is the root, incomplete programs are inner nodes, and 
complete programs are leaves.

## What's the key problem?
We all know that the space of possible programs is huge. However, the majority of them, maybe 
more than 99.99% of them, are incorrect, especially when there are many examples. The problem 
of the naive way is that, before reaching a leaf node, we are totally blind. We don't even have 
the slightest hint whether we are in a correct direction. Several recent papers have proposed 
methods using machine learning to improve the performance. Experiences from people writing 
programs in the same language can be a hint on the correct direction. But I'm using a different 
strategy here.

## An observation
As is mentioned(emphasized) before, the majority of programs are incorrect. It is highly possible 
that a subtree at a very high level(maybe even one of root's children) contains no valid programs 
at all.
We don't want to waste any time in such branches!
If we can find a way to tell "this subtree contains no correct program/at least one correct program"
at the point we reach the subtree's root, we'll be able to drop impossble branches very fast. And 
the less the number of correct programs, the more branches we'll drop, the faster we'll reach a 
correct one.

## How to tell?

### Version Space Algebra
This idea comes from several Microsoft papers, which borrowed the idea from an ancient paper
the name of which I can't remember. The general idea is that a program is the combination of 
sub-programs, and each sub-program has an number of choices. So they use a DAG to represent the 
process of composing a program. Each node in this DAG represent the set of all possible 
sub-programs at some position of the program and satisfying a sub-specification, and its "children" 
are sub-sub-programs that can be used to compose the sub-program itself. Gathering valid sub-programs 
in a divide-and-conquer way will eventually give you a valid program at the top level of the DAG.
One observation here is that in many tasks, not only can we divide the program, but we can 
also divide the specification (what I call "examples" here), and the set of valid programs is 
dependent only on the specification. There may not be as many ways to divide the specification as 
the number of possible programs. Thus we can find some satisfying programs(or conclude there's no
valid program) for each node in the DAG without costing too much time. Beware that this is
exactly the information we want for deciding whether a subtree has at least one correct program.
So we can use it for pruning.

(Why don't we directly use VSA to get a program? This is what Microsoft people did in Flashfill. 
There are mainly 2 problems. First, it can not handle many examples. If there are a huge number (say, 10) 
of examples, the space of specifications explodes too. Second, it can not handle dependent 
sub-programs, e.g. Kleene star, for loop, etc. There can be huge number of correct programs 
(or even infinite) if you collect them bottom-up. Most of them have to be dropped middle-way. 
This is not a problem if sub-programs are independent. However, when sub-programs are dependent, 
you won't be able to tell which is correct until reaching the point you can verify the correctness
of dependents. If you are unlucky, possibly the correct one is already dropped before that point,
and you can never find it back. These two problems can be relatively easily solved with some 
modifications to the algorithm. Details won't be introduced here since I've found a better 
way and the detail is really tedious. This version is finished too (in `search_tree.h/c`) so
you can read the code if you are curious.)

### Partial Execution
When handling regular expression, my modified VSA version has time complexity roughly (more than, actually)
`O(m*k*(n^4))` where m is number of examples, k is number of (complete or incomplete) programs tried, 
and n is the average length of examples. It's not that bad. But still far from satisfactory.
In practice, it can hardly handle examples of length 50, and consumes a lot of memory due to its dynamic
programming nature.

The better way is called Partial Execution. 

The reason we find pruning difficult is that we can not execute an incomplete program and let it 
tell us if it can potentially accept an example directly. We have to somehow collect informations
bottom up until reaching the point we try to prune. VSA greatly compressed the space and information
we need to collect, yet it's still a lot.

But is it really truth that we can not execute an incomplete program? As you may have guessed from
the name, yes, we can ("Ass♂We♂Can!").

We formally define the property "*Partially Executable*": 
- A language L is partially executable if there's a super set of it L',
so that,
- there is a way to replace every non-terminal in L by a complete program in L'
so that,
- given any incomplete program P in L, we can always replace all its non-terminals and get a complete program P' in L'
so that,
- for any specification S, P' satisfies S if and only if P can mutate into a complete program that satisfies S.

The "if and only if" requirement can be loosen to "if". It won't harm correctness, only damage the performance.

Regular expression is partially executable. The super set language is itself.

If we define its syntax like this:
- `<re> :: concat(<clause>, <re>) | <clause>`
- `<clause> :: <star> | <char>`
- `<star> :: (<re>)*`
- `<char> :: . | 0 | 1 | .........`

Then the replacement map is like this:
- `<re> -> .*`
- `<clause> -> .*`
- `<star> -> .*`
- `<char> -> .`

After the replacement, we can directly run the resulting RE on the example for pruning, for which we can use
NFA, which is super fast.

The total time complexity of this algorithm for regular expression is about `O(k*n*p)` where k is number of programs
tested, n is total length of examples, p is size of the program.

For this algorithm to prune well, there are some (reasonable) requirements on the syntax of the language.
- It should be as unambigious as possible. Because the effectiveness of the algorithm is based on the assumption
	that the number of correct program is very small. If the syntax is ambigious, (e.g., writing regular expression
	syntax like this: `<re> :: concat(<re>, <re>) | (<re>)* | . | 0 | 1 | .....`), there will be huge number of correct syntax
	trees for the same program, thus canceling the effect of pruning.
- It should have more levels and use a different non-terminal for a different purpose. Again, this is a bad example:
	`<re> :: concat(<re>, <re>) | (<re>)* | . | 0 | 1 | ......`. It mixes a general regular expression with a clause 
	of it and a kleene star and a character. There are two reasons this is bad. First, we generally want to start searching
	from simpler programs. This kind of syntax fails to tell the potential complexity of an incomplete program.
	Second, the condition of pruning a branch is "it has zero correct program". Using the same non-terminal for multiple
	purposes is like merging multiple subtrees, during which a subtree with correct program will "contaminate" other subtrees
	with no correct program, which makes it difficult to prune at a high level.

