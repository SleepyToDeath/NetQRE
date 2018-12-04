#ifndef FEATURE_VECTOR_HPP
#define FEATURE_VECTOR_HPP

#include <vector>
#include "../data-transducer/op.h"

class FeatureSlot
{
	public:
	FeatureSlot(size_t _size, bool _iterative, unsigned long long _value):
		size(_size),iterative(_iterative),value(_value) { }

	size_t size; /* in bits, aligned to lowest bit */
	bool iterative; /* iterative: no specific value will appear in the program, only iterate through all values, e.g. src_ip; non-iterative: otherwise, e.g. ethernet type */
	unsigned long long value;
};

class CmpFeatureVector: public DT::CmpTagValue
{
	bool operator()(const std::shared_ptr<DT::TagValue> a, const std::shared_ptr<DT::TagValue> b) const {
		auto fa = std::static_pointer_cast<FeatureVector>(a);
		auto fb = std::static_pointer_cast<FeatureVector>(b);
		if (a.size() < b.size())
			return true;
		else if (a.size() > b.size())
			return false;
		else
		{
			for (int i=0; i<a.size(); i++)
				if (a[i] < b[i])
					return true;
				else if (b[i] < a[i])
					return false;
		}
		return false;
	}
};

class FeatureVector
{
	public:
	FeatureVector(std::share_ptr<DT::TagValue> src);
	FeatureVector();

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

