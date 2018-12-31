#include "interpreter.h"
#include "op.hpp"

using std::unique_ptr;
using std::shared_ptr;
using std::string;
using std::vector;
using std::static_pointer_cast;
using std::move;

namespace Netqre {

/* [TODO] modify parser, remove threshold */

std::unique_ptr<IntValue> Machine::process(std::vector<DT::Word> &stream) {


}

void Machine::collect_value_space(std::vector<DT::Word> &stream) {
	
}

shared_ptr<Machine> Interpreter::interpret(std::shared_ptr<NetqreAST> ast) 
{
	auto machine = shared_ptr<Machine>(new Machine());
	real_interpret(ast, machine);
	return machine;
}

void Interpreter::real_interpret(shared_ptr<NetqreAST> ast, shared_ptr<Machine> machine)
{
	switch(ast->type)
	{
		case NetqreExpType::PROGRAM:
		real_interpret(ast->subtree[0], machine);
		real_interpret(ast->subtree[1], machine);
		real_interpret(ast->subtree[2], machine);
		return;

		case NetqreExpType::FILTER:
		machine->filter = ast->subtree[0];
		return;
		
		case NetqreExpType::QRE:
		real_interpret(ast->subtree[0], machine);
		return;

		case NetqreExpType::QRE_NS:
		machine->num_tree = real_interpret_num(ast, machine);
		return;

		case OUTPUT:
		case CONST:
		case THRESHOLD:
		case WILDCARD:
		case UNKNOWN:
	}

}

std::shared_ptr<NumericalTree> real_interpret_num(std::shared_ptr<NetqreAST> ast, std::shared_ptr<Machine> machine);
{
	auto tree = shared_ptr<NumericalTree>(new NumericalTree());
	switch(ast->type)
	{
		case NetqreExpType::QRE_NS:
		tree->is_leaf = false;
		tree->left = real_interpret_num(ast->subtree[0], machine);
		tree->right = real_interpret_num(ast->subtree[1], machine);
		switch(ast->num_type)
		{
			case ADD:
			tree->op = shared_ptr<AddOp>(new AddOp());
			break;

			case SUB:
			tree->op = shared_ptr<SubOp>(new SubOp());
			break;

			case MUL:
			tree->op = shared_ptr<MulOp>(new MulOp());
			break;

			case DIV:
			tree->op = shared_ptr<DivOp>(new DivOp());
			break;
		}
		return tree;

		case NetqreExpType::QRE_VS:
		tree->is_leaf = true;
		tree->leaf = real_interpret_agg(ast);
		machine->qre_list.push_back( tree->leaf );
		return tree;

		default:
		throw string("[real_interpret_num] Shouldn't reach here.");
	}
}

std::shared_ptr<QRELeaf> real_interpret_agg(std::shared_ptr<NetqreAST> ast)
{
	shared_ptr<NetqreAST> cur = ast;
	auto leaf = shared_ptr<QRELeaf>(new QRELeaf());
	while(true)
	{
		switch(cur->type)
		{
			case NetqreExpType::QRE_VS:
			Aggregator agg;
			switch(cur->agg_type)
			{
				case MAX:
				agg->aggop = shared_ptr<MaxOp>(new MaxOp());
				break;

				case MIN:
				agg->aggop = shared_ptr<MinOp>(new MinOp());
				break;

				case SUM:
				agg->aggop = shared_ptr<AddOp>(new AddOp());
				break;

				case AVG:
				throw string("Not supported yet!\n");
			}
			auto feat = cur->subtree[1];
			for (int i = 0; i < feat->subtree.size(); i++)
				agg.param.push_back(feat->subtree[i]->value);
			leaf->agg_stack.push_back(agg);

			cur = cur->subtree[0];
			break;

			case NetqreExpType::QRE_PS:
			leaf->transducer = real_interpret_qre(cur->subtree[0]);
			return leaf;
		}
	}
}

shared_ptr<DT::Transducer> Interpreter::real_interpret_qre(std::shared_ptr<NetqreAST> ast)
{
	auto parse_aggop = [](shared_ptr<NetqreAST> ast) -> shared_ptr<MergeIntOp> {
		switch(ast->agg_type)
		{
			case MAX:
			return shared_ptr<MaxOp>(new MaxOp());

			case MIN:
			return shared_ptr<MinOp>(new MinOp());

			case SUM:
			return shared_ptr<AddOp>(new AddOp());

			case AVG:
			throw string("Not supported yet!\n");
		}
	}

	switch(ast->type)
	{
		case NetqreExpType::QRE_PS:
		switch(ast->reg_type)
		{
			case RegularOpType::STAR:
			auto aggop = parse_aggop(ast->subtree[1]);
			/* [TODO] */	
			case RegularOpType::CONCAT:
			auto aggop = parse_aggop(ast->subtree[2]);
			/* [TODO] */

		}

		case NetqreExpType::QRE_COND:
		auto dt_re = real_interpret_re(ast->subtree[0]);
		/* [TODO] */
	}
	
}


shared_ptr<DT::Transducer> Interpreter::real_interpret_re(std::shared_ptr<NetqreAST> ast)
{
	switch(ast->reg_type)



}

void Interpreter::collect_predicates(std::shared_ptr<NetqreAST> ast, std::vector<shared_ptr<NetqreAST> > & predicates)
{
	if (ast->type != PREDICATE_SET)
	{
		for (int i=0; i<ast->subtree.size() i++)
			collect_predicates(ast->subtree[i], predicates);
	}
	else
	{
		predicates.push_back(ast);
		return;
	}
}

vector<DT::Word> Interpreter::generate_tags(std::vector<shared_ptr<NetqreAST> > & predicates, TokenStream feature_stream)
{
	vector<DT::Word> tag_stream;
	for (int i=0; i<feature_stream.size(); i++)
	{
		DT::Word w;
		w->val = DataValue::factory->get_instance(DT::UNDEF);
		for (int j=0; j<predicates.size(); j++)
		{
			auto pred_tag = unique_ptr<StateValue>(new StateValue());
			pred_tag->active = satisfy(predicates[j], feature_stream[i])
			w->tag_bitmap.push_back(move(pred_tag));
		}
		tag_stream.push_back(w);
	}
	return tag_stream;
}

unique_ptr<BoolValue> Interpreter::satisfy(shared_ptr<NetqreAST> predicate, const FeatureVector & fv)
{
	switch(predicate->bool_type)
	{
		case BoolOpType::OR
		return OrOp::eval(satisfy(predicate->subtree[0], fv), satisfy(predicate->subtree[1], fv));

		case BoolOpType::AND:
		return AndOp::eval(satisfy(predicate->subtree[0], fv), satisfy(predicate->subtree[1], fv));

		case BoolOpType::NONE:
		if (predicate->subtree.size() == 1)
		{
			auto unknown = unique_ptr<BoolValue> (new BoolValue());
			unknown->unknown = true;
			unknown->val = true;
			return unknown;
		}
		else
		{
			auto l = predicate->subtree[0];
			auto r = predicate->subtree[1];
			int index = l->value;
			unsigned long long value = r->value;
			auto sat = unique_ptr<BoolValue> (new BoolValue());
			sat->unknown = false;
			sat->val = (fv[index].value == value);
			return sat;
		}
		
	}
}

}
