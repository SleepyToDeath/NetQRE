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

	/* 	Define values for undefined and conflict. 
		All valid values must be non-negtive */
	enum DataType{
		UNDEF = 1, CONF = 2, VALID = 0;
	};

	class DataValueFactory;
	class DataValue 
	{
		public:
		DataValue(shared_ptr<DataValue> src);
		DataValue(DataType t);
		DataType type;
		static unique_ptr<DataValueFactory> factory;
	};

	class DataValueFactory
	{
		public:
		virtual unique_ptr<DataValue> get_instance(DataType t) = 0;
		virtual unique_ptr<DataValue> get_instance(unique_ptr<DataValue> src) = 0;
	}

	class Word
	{
		share_ptr<TagValue> key;
		share_ptr<DataValue> val;
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
		Do union operation.
		Default behavior follows the specification in paper.
		However, it is allowed to derive this class and do something different 
			(e.g. resolve the conflict)	if you know its cost.
	*/
	class UnionOp : public Op
	{
		public:
		virtual unique_ptr<DataValue> operator ()(
			const vector< unique_ptr<DataValue> > &param, 
			const unique_ptr<DataValue> &current);
	}

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

	class MergeParallelOp : public BasicBinaryOp
	{
		public:
		virtual unique_ptr<DataValue> operator ()(
			const vector< unique_ptr<DataValue> > &param, 
			const unique_ptr<DataValue> &current) = 0;
	}

}

#endif
