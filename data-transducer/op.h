#ifndef _DT_OP_H
#define _DT_OP_H

#include <memory>
#include <vector>

using std::shared_ptr;
using std::unique_ptr;
using std::vector;
using std::move;

/* syntactic sugar */
#define copy_data(x) unique_ptr<DT::DataValue>(DT::DataValue::factory->get_instance(x))

namespace DT
{

	typedef TagType int

/*
	class TagValueFactory;
	class TagValue
	{
		public:
		TagValue(share_ptr<TagValue> src);
		TagValue();
		static unique_ptr<TagValueFactory> factory;
	};

	class CmpTagValue 
	{
		virtual bool operator()(const std::shared_ptr<TagValue> a, const std::shared_ptr<TagValue> b) const = 0;
	};

	class TagValueFactory
	{
		public:
		virtual unique_ptr<TagValue> get_instance() = 0;
		virtual unique_ptr<TagValue> get_instance(unique_ptr<TagValue> src) = 0;
	};
*/

	/* 	Define values for undefined and conflict. 
		All valid values must be non-negtive */
	enum DataType{
		UNDEF = 1, CONF = 2, VALID = 0;
	};

	class DataValueFactory;
	class DataValue 
	{
		public:
		DataValue(const unique_ptr<DataValue> &src);
		DataValue(DataType t);
		DataType type;
		static unique_ptr<DataValueFactory> factory;
	};

	class DataValueFactory
	{
		public:
		virtual unique_ptr<DataValue> get_instance(DataType t) = 0;
		virtual unique_ptr<DataValue> get_instance(unique_ptr<DataValue> src) = 0;
	};

	class Word
	{
		/*
			Each data word may contain multiple tags, e.g., 
			a packet may satisfy multiple predicates in NetQRE. 
			Since usually there should be a small fixed number 
			of tags, we enforce that each tag be converted to an int. 
			The mapping between the real tag and the int should
			be maintained by user. This bitmap should specify
			which tags this data word contains.
		*/
		std::vector<bool> tag_bitmap;
		shared_ptr<DataValue> val;
	};

	/*
		All true operators must be subclass of op
		and override () operator.
	*/
	class Op
	{
		public:
		virtual unique_ptr<DataValue> operator ()(
			const vector< unique_ptr<DataValue> > &param, 
			const unique_ptr<DataValue> &current) = 0;
	};

	/* always keep current value, ignore all input */
	class ConstOp : public Op
	{
		public:
		unique_ptr<DataValue> operator ()(
			const vector< unique_ptr<DataValue> > &param, 
			const unique_ptr<DataValue> &current);
	};

	/* always copy the first value in input */
	class CopyOp : public Op
	{
		public:
		unique_ptr<DataValue> operator ()(
			const vector< unique_ptr<DataValue> > &param, 
			const unique_ptr<DataValue> &current);
	};

	/*
		Do binary operation.
		Derive this class to implement your own operation.
		Should follow the specification in paper 
			when handling conflict and undefined.
	*/
	class BasicBinaryOp : public Op
	{
		public:
		virtual unique_ptr<DataValue> operator ()(
			const vector< unique_ptr<DataValue> > &param, 
			const unique_ptr<DataValue> &current) = 0;
	}

	/* 
		Generalized union.
		Can be default UnionOp.
		Can also be customized to resolve conflicts.
		current is assumed to be nullptr.
	*/
	class MergeParallelOp : public BasicBinaryOp
	{
		public:
		virtual unique_ptr<DataValue> operator ()(
			const vector< unique_ptr<DataValue> > &param, 
			const unique_ptr<DataValue> &current
		) = 0;
	}

	/*	
		Do union operation.
		Default merging behavior.
		Follows the specification in paper.
	*/
	class UnionOp : public MergeParallelOp
	{
		public:
		virtual unique_ptr<DataValue> operator ()(
			const vector< unique_ptr<DataValue> > &param, 
			const unique_ptr<DataValue> &current);
	}
}

#endif
