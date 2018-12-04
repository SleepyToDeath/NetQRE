#ifndef NETQRE2DT_OP_HPP
#define NETQRE2DT_OP_HPP

#include "../data-transducer/op.h"

using std::unique_ptr;
using std::shared_ptr;
using std::string;
using std::vector;
using std::static_pointer_cast;

namespace Netqre {

/* [TODO] merge boolean with int */

class DataValueFactory
{
	public:
	unique_ptr<DataValue> get_instance(DataType t)
	{
		if (t == DT::VALID)
		{
			throw string("This function shouldn't be called\n");
			return nullptr;
		}
		else
		{
			auto data = unique_ptr<BoolType>(new BoolType());
			data->type = t;
			return data;
		}
	}
	
	unique_ptr<DataValue> get_instance(const unique_ptr<DataValue> &src) 
	{
		if (static_pointer_cast<DataType>(src)->is_int)
		{
			auto data = unique_ptr<IntType>(new IntType());
			data->upper = static_pointer_cast<IntType>(src)->upper;
			data->lower = static_pointer_cast<IntType>(src)->lower;
			return data;
		}
		else
		{
			auto data = unique_ptr<BoolType>(new BoolType());
			data->unknown = static_pointer_cast<BoolType>(src)->unknown;
			data->val = static_pointer_cast<BoolType>(src)->val;
			return data;
		}
	}
};

/* int or bool */
class DataType: public DT::DataValue
{
	public:
	bool is_int;
	virtual void dummy() = 0;
};


/* 
	ternary boolean 
	any: true, any
	true: false, true
	false: false, false
*/
class BoolType: public DataType
{
	public:
	BoolType() {
		type = DT::VALID;
		is_int = false;
		unknown = true;
		val = false;
	}

	void dummy() {}

	bool unknown;
	bool val;
};

/*
	interval
	non-negative values
	upper == lower for determined integer
*/
class IntType: public DataType
{
	public:
	IntType() {
		type = DT::VALID;
		is_int = true;
		upper = 0;
		lower = 0;
	}

	void dummy() {}

	int upper;
	int lower;
};

/* bool X bool -> bool */
class OrOp: public DT::MergeParallelOp
{
	const bool unknown_table[][] = {
		{true,	true,	true,	false },
		{true,	true,	true,	false },
		{true,	true,	false,	false },
		{false,	false,	false,	false },
	};

	const bool truth_table[][] = {
		{true,	true,	true,	true },
		{true,	true,	true,	true },
		{true,	true,	false,	true },
		{true,	true,	true,	true },
	};

	public:
	unique_ptr<DataValue> operator ()(
		const vector< unique_ptr<DataValue> > &param, 
		const unique_ptr<DataValue> &current) 
	{
		unique_ptr<BoolType> & a = static_pointer_cast<BoolType>(param[0]);
		unique_ptr<BoolType> & b = static_pointer_cast<BoolType>(param[1]);
		auto ans = unique_ptr<BoolType>(new BoolType());
		size_t x = (((size_t)a->unknown)<<1) | (size_t)a->val;
		size_t y = (((size_t)b->unknown)<<1) | (size_t)b->val;
		ans->unknown = unknown_table[x][y];
		ans->val = truth_table[x][y];
		return ans;
	}
};

/* bool -> int */
class CondOp: public DT::Op
{
	public:
	unique_ptr<DataValue> operator ()(
		const vector< unique_ptr<DataValue> > &param, 
		const unique_ptr<DataValue> &current) 
	{
		unique_ptr<BoolType> & cond = static_pointer_cast<BoolType>(param[0]);
		auto ans = unique_ptr<IntType>(new IntType());
		if (cond->unknown)
		{
			ans->upper = 1;
			ans->lower = 0;
		}
		else
		{
			ans->upper = cond->val?1:0;
			ans->lower = cond->val?1:0;
		}
		return ans;
	}

};

/* int X int -> int */
class AddOp: public DT::BasicBinaryOp
{
	public:
	unique_ptr<DataValue> operator ()(
		const vector< unique_ptr<DataValue> > &param, 
		const unique_ptr<DataValue> &current) 
	{
		unique_ptr<IntType> & a = static_pointer_cast<IntType>(param[0]);
		unique_ptr<IntType> & b = static_pointer_cast<IntType>(param[1]);
		auto ans = unique_ptr<IntType>(new IntType());
		ans->lower = a->lower + b->lower;
		ans->upper = a->upper + b->upper;
		return ans;
	}
};

/* int X int -> int */
/* [?] should we use conflict or undefined ? */
/* [?] should lower bound < 0 be allowed? */
class SubOp: public DT::BasicBinaryOp
{
	public:
	unique_ptr<DataValue> operator ()(
		const vector< unique_ptr<DataValue> > &param, 
		const unique_ptr<DataValue> &current) 
	{
		unique_ptr<IntType> & a = static_pointer_cast<IntType>(param[0]);
		unique_ptr<IntType> & b = static_pointer_cast<IntType>(param[1]);
		auto ans = unique_ptr<IntType>(new IntType());
		ans->lower = a->lower - b->upper;
		ans->upper = a->upper - b->lower;
		if (ans->upper < 0)
			ans->type = DT::CONF;
		if (ans->lower < 0)
			ans->lower = 0;
		return ans;
	}
};

/* int X int -> int */
class MulOp: public DT::BasicBinaryOp
{
	public:
	unique_ptr<DataValue> operator ()(
		const vector< unique_ptr<DataValue> > &param, 
		const unique_ptr<DataValue> &current) 
	{
		unique_ptr<IntType> & a = static_pointer_cast<IntType>(param[0]);
		unique_ptr<IntType> & b = static_pointer_cast<IntType>(param[1]);
		auto ans = unique_ptr<IntType>(new IntType());
		ans->lower = a->lower * b->lower;
		ans->upper = a->upper * b->upper;
		return ans;
	}
};

/* int X int -> int */
/* [?] should div 0 output inf or error ? */
class DivOp: public DT::BasicBinaryOp
{
	public:
	unique_ptr<DataValue> operator ()(
		const vector< unique_ptr<DataValue> > &param, 
		const unique_ptr<DataValue> &current) 
	{
		unique_ptr<IntType> & a = static_pointer_cast<IntType>(param[0]);
		unique_ptr<IntType> & b = static_pointer_cast<IntType>(param[1]);
		auto ans = unique_ptr<IntType>(new IntType());
		if (b->lower == 0)
			ans->type = DT::CONF;
		else
		{
			ans->lower = a->lower / b->upper;
			ans->upper = a->upper / b->lower;
		}
		return ans;
	}
};

}



#endif
