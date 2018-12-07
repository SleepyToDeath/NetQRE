#ifndef NETQRE2DT_OP_HPP
#define NETQRE2DT_OP_HPP

#include "../data-transducer/op.h"

using std::unique_ptr;
using std::shared_ptr;
using std::string;
using std::vector;
using std::static_pointer_cast;

namespace Netqre {

/* the only interface exposed to DT is StateValue */

class DataValueFactory
{
	public:
	unique_ptr<DataValue> get_instance(DT::DataType t)
	{
		if (t == DT::VALID)
		{
			throw string("This function shouldn't be called\n");
			return nullptr;
		}
		else
		{
			auto data = unique_ptr<StateValue>(new StateValue());
			data->type = t;
			return data;
		}
	}
	
	unique_ptr<DataValue> get_instance(const unique_ptr<DataValue> &src) 
	{
		switch(static_pointer_cast<DataValue>(src)->sub_type)
		{
			case DataType::INT:
			auto a = static_pointer_cast<IntValue>(src)
			auto data = unique_ptr<IntValue>(new IntValue(a));
			return data;
			case DataType::BOOL:
			auto a = static_pointer_cast<BoolValue>(src)
			auto data = unique_ptr<BoolValue>(new BoolValue(a));
			return data;
			case DataType::STATE:
			auto a = static_pointer_cast<StateValue>(src)
			auto data = unique_ptr<StateValue>(new StateValue(a));
			return data;
		}
	}
};

enum class DataType { BOOL, INT, STATE };

/* int or bool */
class DataValue: public DT::DataValue
{
	public:
	DataType sub_type;
	virtual void dummy() = 0;
};


/* 
	ternary boolean 
	any: true, any
	true: false, true
	false: false, false
*/
class BoolValue: public DataValue
{
	public:
	BoolValue() {
		type = DT::VALID;
		sub_type = DataType::BOOL;
		unknown = true;
		val = false;
	}

	BoolValue(const unique_ptr<BoolValue>& src)
	{
		type = src->type;
		sub_type = src->sub_type;
		unknown = src->unknown;
		val = src->val;
	}

	void dummy() {}

	bool unknown;
	bool val;
};

/*
	interval
	non-negative values
	upper == lower for determined integer
	both bounds will be automatically raised to 0
	if becoming negative during any computation
*/
class IntValue: public DataValue
{
	public:
	IntValue() {
		type = DT::VALID;
		sub_type = DataType::INT;
		upper = 0;
		lower = 0;
	}

	IntValue(const unique_ptr<IntValue>& src)
	{
		type = src->type;
		sub_type = src->sub_type;
		upper = src->upper;
		lower = src->lower;
	}

	void dummy() {}

	int upper;
	int lower;
};

/* Each aggregation should push a register to value_stack and an aggop
	to op_stack. Higher level of expression should stay at lower position
	in the stack. The top of value_stack should be the conditional output.
	At each transition, register values from different states at the same 
	level of stack will	be merged by aggop.
	At each checkpoint, value_stack[top] and value_stack[top-1] from the
	current state will be merged to value_stack[top-1] by aggop. 
	Both stacks will pop. */
class StateValue: public DataValue
{
	public:
	StateValue() {
		type = DT::VALID;
		sub_type = DataType::STATE;
		active = unique_ptr<BoolValue>(new BoolValue());
		value_stack.clear();
		op_stack.clear();
	}

	StateValue(const unique_ptr<StateValue>& src) {
		type = src->type;
		sub_type = src->sub_type;
		active = unique_ptr<BoolValue>(new BoolValue(src->active));
		for (int i=0; i<src->value_stack.size(); i++)
			value_stack.push_back(unique_ptr<IntValue>(new IntValue(src->value_stack[i])));
		op_stack = src->op_stack;
	}

	vector<unique_ptr<IntValue> > value_stack;
	vector<shared_ptr<DT::Op> op_stack;
	unique_ptr<BoolValue> active;
};

/* StateValue X StateValue -> StateValue 
	(state) 	(predicate)    (state)*/
class PredicateOp: public DT::BasicBinaryOp
{
	public:

	unique_ptr<DataValue> operator ()(
		const vector< unique_ptr<DataValue> > &param, 
		const unique_ptr<DataValue> &current) 
	{
		unique_ptr<StateValue> & a = static_pointer_cast<StateValue>(param[0]);
		unique_ptr<StateValue> & b = static_pointer_cast<StateValue>(param[1]);
		return eval(a, b);
	}

	static unique_ptr<StateValue> eval(const unique_ptr<StateValue>& a, const unique_ptr<StateValue> & b)
	{
		if (!(a->type==DT::VALID && b->type==DT::VALID))
			return DT::DataValue::factory->get_instance(DT::UNDEF);

		auto c = AndOp::eval(a->active, b->active);
		if (c->unknown || c->val)
		{
			auto ans = unique_ptr<StateValue>(new StateValue(a));
			ans->active = unique_ptr<BoolValue>(new BoolValue(c));
			return ans;
		}
		else
			return DT::DataValue::factory->get_instance(DT::UNDEF);
	}
};

/* StateValue X StateValue -> StateValue */
class TransitionOp: public DT::MergeParallelOp
{
	public:

	unique_ptr<DataValue> operator ()(
		const vector< unique_ptr<DataValue> > &param, 
		const unique_ptr<DataValue> &current) 
	{
		unique_ptr<StateValue> & a = static_pointer_cast<StateValue>(param[0]);
		unique_ptr<StateValue> & b = static_pointer_cast<StateValue>(param[1]);
		return eval(a, b);
	}

	static unique_ptr<StateValue> eval(const unique_ptr<StateValue>& a, const unique_ptr<StateValue> & b)
	{
		if (!a->type == DT::VALID)
			return copy_data(b);
		if (!b->type == DT::VALID)
			return copy_data(a);

		auto c = OrOp::eval(a->active, b->active);
		if (c->unknown || c->val)
		{
			auto ans = unique_ptr<StateValue>(new StateValue(a));
			for (int i=0; i<op_stack.size
			ans->active = unique_ptr<BoolValue>(new BoolValue(c));
			return ans;
		}
		else
			return DT::DataValue::factory->get_instance(DT::UNDEF);
};

class PushStackOp: public DT::Op
{


};

class PushMaxOp: public PushStackOp
{

};

class PushMinOp: public PushStackOp
{

};

class PushSumOp: public PushStackOp
{

};

/* [TODO] AVG requires 2 states 
	(number of iterations & sum)
	currently not supported */

class PopStackOp: public DT::Op
{

};

/* bool X bool -> bool */
class AndOp: public DT::MergeParallelOp
{
	const bool unknown_table[][] = {
		{true,	true,	false,	true },
		{true,	true,	false,	true },
		{false,	false,	false,	false },
		{true,	true,	false,	false },
	};

	const bool truth_table[][] = {
		{true,	true,	false,	true },
		{true,	true,	false,	true },
		{false,	false,	false,	false },
		{true,	true,	false,	true },
	};

	public:
	unique_ptr<DataValue> operator ()(
		const vector< unique_ptr<DataValue> > &param, 
		const unique_ptr<DataValue> &current) 
	{
		unique_ptr<BoolValue> & a = static_pointer_cast<BoolValue>(param[0]);
		unique_ptr<BoolValue> & b = static_pointer_cast<BoolValue>(param[1]);
		return eval(a, b);
	}

	static unique_ptr<BoolValue> eval(const unique_ptr<BoolValue>& a, const unique_ptr<BoolValue>& b)
	{
		auto ans = unique_ptr<BoolValue>(new BoolValue());
		size_t x = (((size_t)a->unknown)<<1) | (size_t)a->val;
		size_t y = (((size_t)b->unknown)<<1) | (size_t)b->val;
		ans->unknown = unknown_table[x][y];
		ans->val = truth_table[x][y];
		return ans;
	}
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
		unique_ptr<BoolValue> & a = static_pointer_cast<BoolValue>(param[0]);
		unique_ptr<BoolValue> & b = static_pointer_cast<BoolValue>(param[1]);
		return eval(a, b);
	}

	static unique_ptr<BoolValue> eval(const unique_ptr<BoolValue>& a, const unique_ptr<BoolValue>& b)
	{
		auto ans = unique_ptr<BoolValue>(new BoolValue());
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
		unique_ptr<BoolValue> & cond = static_pointer_cast<BoolValue>(param[0]);
		return eval(cond);
	}

	static unique_ptr<IntValue> eval(const unique_ptr<BoolValue>& cond);
	{
		auto ans = unique_ptr<IntValue>(new IntValue());
		if (cond->unknown)
		{
			ans->upper = 1;
			ans->lower = 1;
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
		unique_ptr<IntValue> & a = static_pointer_cast<IntValue>(param[0]);
		unique_ptr<IntValue> & b = static_pointer_cast<IntValue>(param[1]);
		return eval(a, b);
	}

	static unique_ptr<IntValue> eval(const unique_ptr<IntValue> &a, const unique_ptr<IntValue>& b)
	{
		auto ans = unique_ptr<IntValue>(new IntValue());
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
		unique_ptr<IntValuValue> & a = static_pointer_cast<IntValue>(param[0]);
		unique_ptr<IntValue> & b = static_pointer_cast<IntValue>(param[1]);
		return eval(a, b);
	}

	static unique_ptr<IntValue> eval(const unique_ptr<IntValue> &a, const unique_ptr<IntValue>& b)
	{
		auto ans = unique_ptr<IntValue>(new IntValue());
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
		unique_ptr<IntValue> & a = static_pointer_cast<IntValue>(param[0]);
		unique_ptr<IntValue> & b = static_pointer_cast<IntValue>(param[1]);
		return eval(a, b);
	}

	static unique_ptr<DataValue> eval(const unique_ptr<IntValue> &a, const unique_ptr<IntValue>& b)
	{
		auto ans = unique_ptr<IntValue>(new IntValue());
		ans->lower = a->lower * b->lower;
		ans->upper = a->upper * b->upper;
		return ans;
	}
};

/* int X int -> int */
/* [?] should div 0 output inf or error ? */
/* 		and if error, undef or conf ? */
class DivOp: public DT::BasicBinaryOp
{
	public:
	unique_ptr<DataValue> operator ()(
		const vector< unique_ptr<DataValue> > &param, 
		const unique_ptr<DataValue> &current) 
	{
		unique_ptr<IntValue> & a = static_pointer_cast<IntValue>(param[0]);
		unique_ptr<IntValue> & b = static_pointer_cast<IntValue>(param[1]);
		return eval(a, b);
	}

	static unique_ptr<DataValue> eval(const unique_ptr<IntValue> &a, const unique_ptr<IntValue>& b)
	{
		auto ans = unique_ptr<IntValue>(new IntValue());
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
