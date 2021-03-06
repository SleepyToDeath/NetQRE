#ifndef FEATURE_VECTOR_HPP
#define FEATURE_VECTOR_HPP

#include <vector>
//#include "../data-transducer/op.h"
#include "rubify.hpp"

using Rubify::vector;

typedef long long StreamFieldType;

typedef StreamFieldType FeatureSlot;

typedef vector<FeatureSlot> FeatureVector;

typedef vector<FeatureVector> TokenStream;

enum class StreamFieldProperty {
	SCALAR = 0, RANGED = 1, DISCRETE = 2
};

class StreamConfig
{
	public:
	/* number of fields in each vector */
	int field_number;

	/* in bits, aligned to lowest bit */
	vector<size_t> field_size; 
	
	/* iterative: no specific value will appear in the program, 
		only iterate through all values, e.g. src_ip; 
		non-iterative: otherwise, e.g. ethernet type */
	vector<StreamFieldProperty> field_property;
};


/* ============================ unnecessary ==================================  */
/*
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
*/

/*
class FeatureVector
{
	public:
	FeatureVector() {};
//	FeatureVector(const FeatureVector& src):features(src.features) { }

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
*/


#endif
