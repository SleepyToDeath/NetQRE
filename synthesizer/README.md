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
							A smaller number will increase the chance to find an answer early, but also slows down the search in general.
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

Notice that you should use the CSV files in `GeneratedLabelledFlows.zip`, NOT `MachineLearningCSV.zip`.

Place all pcap files in `./testbed/TrafficLabelling/dataset/`

Place all csv files in `./testbed/TrafficLabelling/csv/`

Now you can goto `./testbed/TrafficLabelling/` and use `mixer.rb` to generate positive/negative 
training/testing data. For each attack type, generate one positive training file and one positive 
testing file. Generate one shared negative training file and one shared negative testing file.
`mixer.rb` contains one configuration for each file to generate. Uncomment the target config
and comment all others and run `mixer.rb` to generate the corresponding file(printed to stdout; 
log is printed in stderr so you can safely redirect stdout to a file).
Use only targets in `for CICIDS2017` section.

Notice that this step may have aggressive usage of memory. You'll probably need 32GB of memory for it to
finish normally.

Then you can use the workflow above to learn classifier programs and test them. 
Learn one attack type each time. You'll need 4 files for each (pos/neg train/test).

