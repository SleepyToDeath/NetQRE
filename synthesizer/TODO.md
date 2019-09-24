#Once Upon a Time:

## Speculative vs exhaustive search
	- speculative: use positive and negative examples with same index as basic search unit, `n` units in total
	- exhaustive: use each pair of positive and negative examples as basic search unit, `n^2` units in total
	- currently implemented speculative one

## Add compare primitive
	- `[ field > value ]`
	- `[ field < value ]`
	- How to extract candidate fields and values??/


## Use temporal information as feature, express trending

### need to express:
	number of instances such that
	{ 
		{ 
			time between two consecutive occurance of {some pattern} 
		} 
		is greater/less than some value 
	} 
	is greater than some value

### what it means?
	classify traces based on the frequency of {some pattern} 

### How:
	1. add a preprocessor that filters packet by {some pattern} (How???)
	2. add a field of {time to next packet} to each packet
	3. then we can use compare primitive [ {time to next packet} > / < some value ] to identify satisfying instances of time between two consecutive occurance of {some pattern}
	4. then gather by counting `iter(/ _* [ {time to next packet} > / < some value ] _* /?1, sum)`

### why it finds trending?
	- trending: something happens more/less frequently recently
	- positive example: recent traces
	- negative example: long ago traces
	- if there indeed is a trending, recent traces should be classified as positive





#2019/09/22

## wrap pred with skip [DONE]
- `*(_) [pred] *(_)` [`[pred]*(_)`]
- default or optional ? [default]
- try both

## take max value of output [DONE]

## search over different scales
- concatenate every n flows as a new example
- n = 2^k, k = 0, 1, 2, 3,.....
- how to convert the pattern back for original examples?
- what do we eventually want?

## prioritize harvested subtree
- a complexity reward to grammar RHS

## make the search more deterministic [future work]
- start with low accuracy requirement?
- break down grammar?
- break down examples?
- module?

