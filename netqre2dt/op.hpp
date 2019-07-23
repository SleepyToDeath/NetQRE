#ifndef NETQRE2DT_OP_HPP
#define NETQRE2DT_OP_HPP

#include "../data-transducer/op.h"
#include "syntax.h"
#include <sstream>
#include "rubify.hpp"

using Rubify::string;
using Rubify::vector;
using Rubify::map;
using std::getline;
using std::stringstream;
using std::unique_ptr;
using std::shared_ptr;
using std::static_pointer_cast;

#define copy_typed_data(T, x) std::unique_ptr<T>(new T((T*)(x.get())))

namespace Netqre {

/*
class DataValue;
class StateValue;
class BoolValue;
class IntValue;
class DataValueFactory;
*/


/* ============ Data Type Define ===============*/

enum class DataType { BOOL, INT, STATE };

/* int or bool */
class DataValue: public DT::DataValue
{
	public:
	DataType sub_type;
	virtual ~DataValue() {};
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

	BoolValue(const unique_ptr<BoolValue>& src):BoolValue(src.get()) {}

	BoolValue(const BoolValue* src)
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

	IntValue(const unique_ptr<IntValue>& src):IntValue(src.get()) {}

	IntValue(const IntValue* src)
	{
		type = src->type;
		sub_type = src->sub_type;
		upper = src->upper;
		lower = src->lower;
	}

	IntValue(StreamFieldType val):upper(val),lower(val)
	{
		type = DT::VALID;
		sub_type = DataType::INT;
	}

	void dummy() {}

	StreamFieldType upper;
	StreamFieldType lower;

	const static StreamFieldType MAXIMUM = 1<<20;
};

/* Each aggregation should push a register to value_stack
	Higher level of expression should stay at lower position
	in the stack. The top of value_stack should be the 
	conditional output.	At each transition, register values 
	from different states at the same level of stack will be 
	merged by MergeOp.
	At each checkpoint, value_stack[top] and value_stack[top-1]
	from the current state will be merged to value_stack[top-1]. 
	value_stack will pop. */
class StateValue: public DataValue
{
	public:
	StateValue() {
		type = DT::VALID;
		sub_type = DataType::STATE;
		active = unique_ptr<BoolValue>(new BoolValue());
		value_stack.clear();
//		op_stack.clear();
	}

	std::string to_string() {
		switch (type)
		{
			case DT::UNDEF:
			return string("Undefined");

			case DT::CONF:
			return string("Conflict");

			case DT::VALID:
			{
				stringstream ss;
				ss<< "Stack: size:"<<value_stack.size();
				for (int i=0; i<value_stack.size(); i++)
					ss<<" | "<<value_stack[i]->lower<<"~"<<value_stack[i]->upper;
				if (active->unknown)
					ss<<" | unknown";
				else if (active->val)
					ss<<" | true";
				else
					ss<<" | false";
				string ans;
				getline(ss, ans);
				return ans;
			}

			default:
			return string("Unknown type");
		}
	}

	StateValue(const unique_ptr<StateValue>& src):StateValue(src.get()) { }

	StateValue(const StateValue& src):StateValue(&src) { }

	StateValue(const StateValue* src) {
		type = src->type;
		if (type == DT::VALID)
		{
			sub_type = src->sub_type;
			active = unique_ptr<BoolValue>(new BoolValue(src->active));
			for (int i=0; i<src->value_stack.size(); i++)
				value_stack.push_back(unique_ptr<IntValue>(new IntValue(src->value_stack[i])));
		}
	}

	void dummy() {}

	vector<unique_ptr<IntValue> > value_stack;
//	vector<shared_ptr<DT::Op> op_stack;
	unique_ptr<BoolValue> active;
};


//		virtual unique_ptr<DataValue> get_instance(DataType t) = 0;
//		virtual unique_ptr<DataValue> get_instance(const unique_ptr<DataValue>& src) = 0;

class DataValueFactory: public DT::DataValueFactory
{
	public:
	unique_ptr<DT::DataValue> get_instance(DT::DataType t)
	{
		return real_get_instance(t);
	}
	
	static unique_ptr<StateValue> real_get_instance(DT::DataType t)
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
	
	unique_ptr<DT::DataValue> get_instance(const unique_ptr<DT::DataValue> &src) 
	{
		return get_instance(src.get());
	}

	static unique_ptr<DT::DataValue> get_instance(const DT::DataValue* src) 
	{
		if (src->type != DT::VALID)
			return real_get_instance(src->type);

		switch(((DataValue*)src)->sub_type)
		{
			case DataType::INT:
			{
				auto a = (IntValue*)src;
				auto data = unique_ptr<IntValue>(new IntValue(a));
				return data;
			}
			case DataType::BOOL:
			{
				auto a = (BoolValue*)src;
				auto data = unique_ptr<BoolValue>(new BoolValue(a));
				return data;
			}
			case DataType::STATE:
			{
				auto a = (StateValue*)src;
				auto data = unique_ptr<StateValue>(new StateValue(a));
				return data;
			}

			default:
			throw string("Impossible data type!");
		}
	}
};



/* ============ Operator Define ===============*/

/* bool X bool -> bool */
class AndOp: public DT::MergeParallelOp
{
	public:
	const static bool unknown_table[4][4];

	const static bool truth_table[4][4];

	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		auto a = (BoolValue*)(param[0].get());
		auto b = (BoolValue*)(param[1].get());
		return eval(a, b);
	}

	static unique_ptr<BoolValue> eval(const BoolValue* a, const BoolValue* b)
	{
		auto ans = unique_ptr<BoolValue>(new BoolValue());
		size_t x = (((size_t)!(a->unknown))<<1) | (size_t)a->val;
		size_t y = (((size_t)!(b->unknown))<<1) | (size_t)b->val;
		ans->unknown = unknown_table[x][y];
		ans->val = truth_table[x][y];
		return ans;
	}
};

/* bool X bool -> bool */
class OrOp: public DT::MergeParallelOp
{
	public:
	const static bool unknown_table[4][4];

	const static bool truth_table[4][4];

	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		auto a = (BoolValue*)(param[0].get());
		auto b = (BoolValue*)(param[1].get());
		return eval(a, b);
	}

	static unique_ptr<BoolValue> eval(const BoolValue* a, const BoolValue* b)
	{
		auto ans = unique_ptr<BoolValue>(new BoolValue());
		size_t x = (((size_t)!(a->unknown))<<1) | (size_t)a->val;
		size_t y = (((size_t)!(b->unknown))<<1) | (size_t)b->val;
		ans->unknown = unknown_table[x][y];
		ans->val = truth_table[x][y];
		return ans;
	}
};


class MergeIntOp: public DT::MergeParallelOp
{
	public:
	virtual unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current ) = 0;
	virtual unique_ptr<IntValue> eval(const IntValue* a, const IntValue* b) = 0;
};


/* int X int -> int */
class AddOp: public MergeIntOp
{
	public:
	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		IntValue* a = (IntValue*)(param[0].get());
		IntValue* b = (IntValue*)(param[1].get());
		return eval(a, b);
	}

	unique_ptr<IntValue> eval(const IntValue* a, const IntValue* b)
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
class SubOp: public MergeIntOp
{
	public:
	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		IntValue* a = (IntValue*)(param[0].get());
		IntValue* b = (IntValue*)(param[1].get());
		return eval(a, b);
	}

	unique_ptr<IntValue> eval(const IntValue* a, const IntValue* b)
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
class MulOp: public MergeIntOp
{
	public:
	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		IntValue* a = (IntValue*)(param[0].get());
		IntValue* b = (IntValue*)(param[1].get());
		return eval(a, b);
	}

	unique_ptr<IntValue> eval(const IntValue* a, const IntValue* b)
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
class DivOp: public MergeIntOp
{
	public:
	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		IntValue* a = (IntValue*)(param[0].get());
		IntValue* b = (IntValue*)(param[1].get());
		return eval(a, b);
	}

	unique_ptr<IntValue> eval(const IntValue* a, const IntValue* b)
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

/* int X int -> int */
class MaxOp: public MergeIntOp
{
	public:
	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		IntValue* a = (IntValue*)(param[0].get());
		IntValue* b = (IntValue*)(param[1].get());
		return eval(a, b);
	}

	unique_ptr<IntValue> eval(const IntValue* a, const IntValue* b)
	{
		auto max = [](int a, int b) -> int {
			if (a>b)
				return a;
			else
				return b;
		};

		auto ans = unique_ptr<IntValue>(new IntValue());
		ans->lower = max(a->lower, b->lower);
		ans->upper = max(a->upper, b->upper);
		return ans;
	}
};

/* int X int -> int */
class MinOp: public MergeIntOp
{
	public:
	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		IntValue* a = (IntValue*)(param[0].get());
		IntValue* b = (IntValue*)(param[1].get());
		return eval(a, b);
	}

	unique_ptr<IntValue> eval(const IntValue* a, const IntValue* b)
	{
		auto min = [](int a, int b) -> int {
			if (a<b)
				return a;
			else
				return b;
		};

		auto ans = unique_ptr<IntValue>(new IntValue());
		ans->lower = min(a->lower, b->lower);
		ans->upper = min(a->upper, b->upper);
		return ans;
	}
};


/* int X int -> int */
class MergeOp: public MergeIntOp
{
	public:

	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		IntValue* a = (IntValue*)(param[0].get());
		IntValue* b = (IntValue*)(param[1].get());
		return eval(a, b);
	}

	unique_ptr<IntValue> eval(const IntValue* a, const IntValue* b)
	{
		auto max = [](int a, int b) -> int {
			if (a>b)
				return a;
			else
				return b;
		};

		auto min = [](int a, int b) -> int {
			if (a<b)
				return a;
			else
				return b;
		};

		auto ans = unique_ptr<IntValue>(new IntValue());
		ans->lower = min(a->lower, b->lower);
		ans->upper = max(a->upper, b->upper);
		return ans;
	}
};

/* StateValue -> StateValue */
class PopStackOp: public DT::PipelineOp
{
	public:
	PopStackOp(shared_ptr<MergeIntOp> op):the_op(op) { name = "PopStackOp"; }

	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		if (param.size() == 0)
			return copy_data(current);
		if (param[0]->type == DT::UNDEF)
			return DataValueFactory::real_get_instance(DT::UNDEF);
		unique_ptr<StateValue> state = copy_typed_data(StateValue, param[0]);
		auto len = state->value_stack.size();
		unique_ptr<IntValue>& a = state->value_stack[len-1];
		unique_ptr<IntValue>& b = state->value_stack[len-2];
		state->value_stack[len-2] = the_op->eval(a.get(), b.get());
		state->value_stack.pop_back();
		return state;
	}

	private:
	shared_ptr<MergeIntOp> the_op;
};


/* push a pre-set init value to stack
	the value should be set on construction of the op */
/* can be used as operator for input gate, in which case
	value will be pushed to the input */
class PushStackOp: public DT::PipelineOp
{
	public:
	PushStackOp(const unique_ptr<IntValue> &val):init_value(copy_typed_data(IntValue, val)) { name = "PushStackOp"; }

	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		unique_ptr<StateValue> state;

		if (param.size() == 0)
			state = copy_typed_data(StateValue, current);
		else
			state = copy_typed_data(StateValue, param[0]);

		if (state->type == DT::UNDEF)
			return DataValueFactory::real_get_instance(DT::UNDEF);

		state->value_stack.push_back(copy_typed_data(IntValue, init_value));
		return state;
	}

	private:
	unique_ptr<IntValue> init_value;
};

/* StateValue X IntValue -> StateValue */
/*
class PushStackOp: public DT::BasicBinaryOp
{
	unique_ptr<DataValue> operator ()(
		const vector< unique_ptr<DataValue> > &param, 
		const unique_ptr<DataValue> &current) 
	{
		unique_ptr<StateValue> & state = static_pointer_cast<StateValue>(copy_data(param[0]));
		unique_ptr<IntValue> & val = static_pointer_cast<IntValue>(copy_data(param[1]));
		state->value_stack.push_back(val);
		return state;
	}
};
*/

/*
class PushStackOp: public DT::Op
{
	public:
	void push(const std::unique_ptr<StateValue>& state, const std::unique_ptr<IntValue>& val, std::shared_ptr<DT::BasicBinaryOp> op) {
		state->op_stack.push_back(op);
		state->value_stack.push_back(val);
	}
};

class PushMaxOp: public PushStackOp
{
	unique_ptr<DataValue> operator ()(
		const vector< unique_ptr<DataValue> > &param, 
		const unique_ptr<DataValue> &current) 
	{
		unique_ptr<> & a = static_pointer_cast<BoolValue>(param[0]);
		unique_ptr<BoolValue> & b = static_pointer_cast<BoolValue>(param[1]);
		return eval(a, b);
	}
};

class PushMinOp: public PushStackOp
{

};

class PushSumOp: public PushStackOp
{

};
*/

/* [TODO] AVG requires 2 states 
	(iteration count & sum)
	currently not supported */

/* StateValue X StateValue -> StateValue 
	(state) 	(predicate)    (state)*/
class PredicateOp: public DT::MergeParallelOp
{
	public:
	PredicateOp() {name = "PredicateOp"; }

	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		StateValue* a = (StateValue*)(param[0].get());
		StateValue* b = (StateValue*)(param[1].get());
		return eval(a, b);
	}

	static unique_ptr<StateValue> eval(const StateValue* a, const StateValue* b)
	{
		if (!(a->type==DT::VALID && b->type==DT::VALID))
			return DataValueFactory::real_get_instance(DT::UNDEF);

		auto c = AndOp::eval(a->active.get(), b->active.get());
		if (c->unknown || c->val)
		{
			auto ans = unique_ptr<StateValue>(new StateValue(a));
			ans->active = unique_ptr<BoolValue>(new BoolValue(c));
			return ans;
		}
		else
			return DataValueFactory::real_get_instance(DT::UNDEF);
	}
};

/* StateValue X StateValue -> StateValue */
/* [TODO] arbitrary number of inputs */
class TransitionOp: public DT::MergeParallelOp
{
	public:
	TransitionOp() { name = "TransitionOp"; }

	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		StateValue* a = (StateValue*)(param[0].get());
		StateValue* b = (StateValue*)(param[1].get());
		return eval(a, b);
	}

	static unique_ptr<StateValue> eval(const StateValue* a, const StateValue* b)
	{
		if (!(a->type == DT::VALID))
			return unique_ptr<StateValue>(new StateValue(b));
		if (!(b->type == DT::VALID))
			return unique_ptr<StateValue>(new StateValue(a));

		auto c = OrOp::eval(a->active.get(), b->active.get());
		if (c->unknown || c->val)
		{
			auto ans = unique_ptr<StateValue>(new StateValue(a));
			for (int i=0; i<a->value_stack.size(); i++)
			{
				MergeOp op;
				ans->value_stack[i] = op.eval(a->value_stack[i].get(), b->value_stack[i].get());
			}
			ans->active = unique_ptr<BoolValue>(new BoolValue(c));
			return ans;
		}
		else
			return DataValueFactory::real_get_instance(DT::UNDEF);
	}
};

/* bool -> int */
class CondOp: public DT::PipelineOp
{
	public:
	CondOp(){ name = "CondOp"; }

	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		if (param.size() == 0)
			return copy_data(current);
		if (param[0]->type == DT::UNDEF)
			return DataValueFactory::real_get_instance(DT::UNDEF);
		StateValue* cond = (StateValue*)(param[0].get());
		return eval(cond);
	}

	static unique_ptr<StateValue> eval(const StateValue* cond)
	{
		auto val = unique_ptr<IntValue>(new IntValue());
		if (cond->active->unknown)
		{
			val->upper = 1;
			val->lower = 1;
		}
		else if (cond->active->val)
		{
			val->upper = 1;
			val->lower = 1;
		}
		else
		{
			return DataValueFactory::real_get_instance(DT::UNDEF);
		}
		auto ans = unique_ptr<StateValue>(new StateValue(cond));
		ans->value_stack.push_back(move(val));
		return ans;
	}

};

/*
class InitBoolOp: public DT::PipelineOp
{
	public:
	unique_ptr<DT::DataValue> operator ()(
		const vector< unique_ptr<DT::DataValue> > &param, 
		const unique_ptr<DT::DataValue> &current) 
	{
		if (param.size() == 0)
			return DataValueFactory::real_get_instance(DT::UNDEF);
		StateValue* input = (StateValue*)(param[0].get());
		return eval(input);
	}

	static unique_ptr<StateValue> eval(const StateValue* input)
	{
		auto ans = unique_ptr<StateValue>(new StateValue(input));
	}


};
*/

};

/* =================================================================== */




#endif
