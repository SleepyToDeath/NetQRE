# What is this?
* *NetQRE* is a high level networking domain specific programing language for quantitative analysis through regular expressions.
* `compiler-native` contains the original implementation of NetQRE.
* `data-transducer` contains a basic implementation of Data Transducer, a generalized model of NFA which supports quantitative computations.
* `netqre2dt` contains a complier from NetQRE to data-transducer, which is expected to be faster than native implementation.
* `synthesizer` contains a code generator of NetQRE. Different from traditional *Program By Example* or *Program Synthesis*,
	it aims at automatically extracting patterns(in the form of NetQRE) of positive samples based on only *inputs* of positive
	samples and negtive samples, and thus sets human free from many monitoring and analysis works.

