#ifndef FEATURE_VECTOR_HPP
#define FEATURE_VECTOR_HPP

#include <vector>

class FeatureSlot
{
	public:
	FeatureSlot(size_t _size, bool _iterative, unsigned long long _value):
		size(_size),iterative(_iterative),value(_value) { }

	size_t size; /* in bits, aligned to lowest bit */
	bool iterative; /* iterative: no specific value will appear in the program, only iterate through all values, e.g. src_ip; non-iterative: otherwise, e.g. ethernet type */
	unsigned long long value;
};

class FeatureVector
{
	public:
	size_t size() {
		return features.size();
	}

	FeatureSlot& operator [](size_t index) {
		return features[index];
	}

	void push_back(FeatureSlot f) {
		features.push_back(f);
	}

	void pop_back() {
		features.pop_back();
	}

	std::vector<FeatureSlot> features;
};

class TokenStream
{
	public:
	size_t size() {
		return tokens.size();
	}

	FeatureVector& operator [](size_t index) {
		return tokens[index];
	}

	void push_back(FeatureVector f) {
		tokens.push_back(f);
	}

	void pop_back() {
		tokens.pop_back();
	}

	std::vector<FeatureVector> tokens;
};

#endif

