# What's this?
This is a synthesizer for NetQRE programs. It takes labeled network traces as inputs
and generates some candidate programs that can distinguish positive and negative traces.

# How to use?

## Overview
The synthesizer has 2 parts: the enumerator and the execution server. Their functions are
straightforward from the name. There will be an enumerator process and arbitrary number of 
execution servers. Whenever the enumerator wants to check a program, it will send the program to the 
least busy server for execution via RPC.

This directory contains only the code for enumerator. The execution server is located
in `../netqre2dt/` and `../data-transducer/`. Its build directory is `../netqre2dt/rpc`.

## Dependency
- A compiler that supports full feature of c++20
(
You can install g++-9 on older versions of Ubuntu:  
`sudo add-apt-repository ppa:ubuntu-toolchain-r/test`  
`sudo apt-get update`  
`sudo apt-get install g++-9`  
)   

- rpclib (may need manual installation; see `rpclib.net` for more detail; type `sudo make install` after the compile step)

- libpcap, libpcap-dev (core packages available in most linux distro's official repo)

- Rubify (github.com/SleepyToDeath/rubify, place side by side with the top level directory of this repo, if you see strange grammar in the code, it's probably from this library)

## Compile
- `make` here
- `make` in `../netqre2dt/rpc`
- Beware to change `CC` variable in both Makefiles if you do not use `g++`

## Run
To use the system, you need to do the following:
1. prepare configuration files for the enumerator
2. prepare data for the enumerator
3. prepare the same data for execution servers (since transmitting data is expensive, we decide that they should be available when the servers start)
4. start execution servers
5. run the enumerator

Usage of enumerator: 
`synthesizer grammar parameters server_list positive_training_data negative_training_data positive_testing_data negative_testing_data`

Usage of execution server:
`netqre_server positive_training_data negative_training_data positive_testing_data negative_testing_data port_number`

Each of the parameters above is a text file, except for `port_number`, which is an integer.

There are 3 configuration files needed:
1. Grammar: you can directly use `./lang/netqre/netqre.json` as the grammar file.
   It contains the same grammar as the Sigcomm paper, which is a little verbose.
   We are yet to implement a parser for the simplified grammar in the TACAS paper.

2. Hyper parameters: you can directly use `./lang/netqre/netqre.config` as the parameter file.
   The parameters and their usages are as follows:
   - `do_test`: whether the learned programs will be tested on the testing data
   - `search_depth`: the greatest size of programs that can be searched. Could be safely ignored in most cases.
   - `batch_size`: how many new (partial)programs will be explored before doing a sort by complexity.
   				  A greater number will increase the chance to find an answer early but also slows down the search in general.
   - `explore_rate`: how many new (partial)programs do you expect an existing partial program to produce on average.
   				  Will affect the performance in an uncertain way if an inaccurate number is given.
   - `answer_count`: how many different programs do you want to learn. The greater the number is, the longer it will take.
   - `threads`: how many threads would you like to use to enumerate programs. Usually not very important.
   - `minimal_example_size`: in the divide-and-conquer process, 
   							what is the minimal size you want to divide the training data into.
							A smaller number will quickly give partial patterns, but they will also be less accurate.
   - `force_search_factor`: in the divide-and-conquer process, if enough existing candidate programs can
   							correctly classify a new subset, the search over this subset will be skipped.
							You can force a search after a number of skips with this parameter.
							A greater number will increase the chance to find an answer early, but also slows down the search in general.
   - `accuracy`: the accuracy over training set required to consider a candidate program correct. 
   				A higher number will give more accurate answers, but will also take longer to finish.
   - `accuracy_exp`: the actual accuracy you believe an answer program can achieve on average. 
   					Not useful if you expect one program that can do the entire classification job as we did in the TACAS paper.
					Should be the same as `accuracy` in this case. Its true usage will be explained in a latter section.
   - `give_up_count`: the enumerator will give up after searching over this number of programs in total
   - `VERBOSE_MODE`: wheter a lot of debugging messages should be printed

3. Server List: it should start with a line containing the number of execution servers. 
   Then each following line contains a server's url and its port number, separated by a whitespace.
   One example is `./lang/netqre/local_server_list.txt`

All data files share the same structure:  
The first line has two numbers: the total number of traces in this file, and the number of features extracted from each packet.
The second line has the same number of integers as the feature number in line 1, each representing a feature type.

There are three feature types:
- SCALAR = 0, can be compared, and may belong to a certain range, such as sequence number
- RANGED = 1, not comparable, but may belong to a certain range, such as IP address
- DISCRETE = 2, not comparable and not belonging to any range, such as protocol type

Then the file contains the tokenized traces as is indicated in line 1.
Each trace contains arbitrary lines, each representing a packet. A packet
is represented by its features listed in the same order as is indicated in
line 2. There should be an empty line between two traces. 
Each trace is seen as one data point by the synthesizer.
Line 2 should be identical across all data files.

If you do not have test data, just put an empty file with 0 traces there.

You may want to write a script to distribute the data and start the executables. 
One example can be found in the Makefile here under `train-netqre-distro` target and also in the
Makefile and bash scripts in the execution server directory. But since they are for our own cluster,
you will need to modify them to fit your own machine and network setup.

## Interpret the result
The result contains all the answer programs found and their accuracies over training and testing data.
The last number after each program is the true threshold learned for it.
Constants in the programs are represented by position in the value space
(in a binary format. the first number is always 1, then each 0 or 1 means picking the upper or lower half of the current value space. For example `101` means `25% ~ 50%`). 
We are yet to substitute the true numbers back into the programs.
After each accuracy result are the NetQRE program's outputs on all data points, each data point occupying one line.





# Another mode
[TODO] 
This is actually a possible future work.




# Prepare CICIDS2017 Dataset
We use CICIDS2017 dataset as a benchmark to demonstrate the ability of our system. 
It is provided in the format of a CSV file that gives the extracted feature vector 
and label of entire flows and a pcap file that contains the raw trace. 

For a typical machine learning system, it is enough to use the CSV file. But we need 
to correlate the two to rebuild labelled raw traces and turn them into the tokenized
trace file our system takes as input. 

We have a script writen in Ruby that automatically does this job. To use it, you first
need to install the dependencies:
- Ruby
- Ruby packet `pcaprub` (with gem)
- Ruby packet `packetfu` (with gem)

Then download the dataset from https://www.unb.ca/cic/datasets/ids-2017.html

After that, split the pcap files into smaller pcap files with tcpdump, each of
size 300MB. This may take a while.
- `tcpdump -r FileName -w FileName.split -C 300`

Extract positive(attack) entries from the CSV files by using `./testbed/TrafficLabelling/extract.rb`
- `extract.rb FileName`

Place all pcap files in `./testbed/TrafficLabelling/dataset/`

Place all csv files in `./testbed/TrafficLabelling/csv/`

Now you can goto `./testbed/TrafficLabelling/` and use `mixer.rb` to generate positive/negative 
training/testing data. For each attack type, generate one positive training file and one positive 
testing file. Generate one shared negative training file and one shared negative testing file.
`mixer.rb` contains one configuration for each file to generate. Uncomment the target config
and comment all others and run `mixer.rb` to generate the corresponding file.
Use only targets in `for CICIDS2017` section.

Then you can use the workflow above to learn classifier programs and test them. 
Learn one attack type each time. You'll need 4 files for each (pos/neg train/test).



# How the synthesis work?
This part is kind of outdated. You may refer to our paper for more technical details.

The synthesis method should work generally for many languages.
Details of synthesizing NetQRE can be found in our paper. Here I only introduce the 
intuition with regular expression.

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

