#ifndef _DT_OP_H
#define _DT_OP_H

#include <memory>
#include <vector>

//#define DT_DEBUG

using std::shared_ptr;
using std::unique_ptr;
using std::vector;
using std::move;

/* syntactic sugar */
#define copy_data(x) (DT::DataValue::factory->get_instance(x))

typedef int TagType;

namespace DT
{


	/* 	Define values for undefined and conflict. 
		All valid values must be non-negtive */
	enum DataType{
		UNDEF = 1, CONF = 2, VALID = 0
	};

	class DataValue;
	class DataValueFactory
	{
		public:
		virtual ~DataValueFactory();
		virtual unique_ptr<DataValue> get_instance(DataType t) = 0;
		virtual unique_ptr<DataValue> get_instance(const unique_ptr<DataValue>& src) = 0;
	};

	class DataValue 
	{
		public:
//		DataValue(const unique_ptr<DataValue> &src);
//		DataValue(DataType t);
//		DataValue();
		DataType type;

		virtual ~DataValue();
		virtual std::string to_string();
		static std::unique_ptr<DataValueFactory> factory;
	};

	class Word
	{
		public:
		/*
			Each data word may contain multiple tags, e.g., 
			a packet may satisfy multiple predicates in NetQRE. 
			Since usually there should be a small fixed number 
			of tags, we enforce that each tag be converted to an int. 
			The mapping between the real tag and the int should
			be maintained by user. 
			In deterministic situation, this should be a bitmap that 
			specifies which tags this data word contains and a
			value shared to all matching circuits.
			But to support undeterministic cases, it is allowed to
			send different values to different circuits. The tag
			matching result is assumed to be contained in this value.
			All circuits will be activated and receive their 
			corresponding values.
		*/
		std::vector<unique_ptr<DataValue> > tag_bitmap;

		Word();
		Word(const Word &src);
		Word& operator=(const Word &src);
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

		std::string name;
	};

	/* always keep current value, ignore all input */
	class ConstOp : public Op
	{
		public:
		ConstOp() { name = "ConstOp"; }
		unique_ptr<DataValue> operator ()(
			const vector< unique_ptr<DataValue> > &param, 
			const unique_ptr<DataValue> &current);
	};

	/* can accept 0 or 1 input */
	class PipelineOp : public Op
	{
		public:
		virtual unique_ptr<DataValue> operator ()(
			const vector< unique_ptr<DataValue> > &param, 
			const unique_ptr<DataValue> &current) = 0;
	};

	/* If no input, same as const op
		Otherwise copy the first value in input */
	class CopyOp : public PipelineOp
	{
		public:
		CopyOp(){ name = "CopyOp"; }
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
	};

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
	};

	/*	
		Do union operation.
		Default merging behavior.
		Follows the specification in paper.
	*/
	class UnionOp : public MergeParallelOp
	{
		public:
		UnionOp() { name = "UnionOp"; }
		virtual unique_ptr<DataValue> operator ()(
			const vector< unique_ptr<DataValue> > &param, 
			const unique_ptr<DataValue> &current);
	};
}

#endif
