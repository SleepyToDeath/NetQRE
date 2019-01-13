#include "interpreter.h"
#include "op.hpp"
#include <algorithm>
#include <map>
#include <utility>

using std::unique_ptr;
using std::shared_ptr;
using std::string;
using std::vector;
using std::static_pointer_cast;
using std::move;
using std::map;
using std::for_each;
using std::pair;

namespace Netqre {

/* [TODO] modify parser, remove threshold */
/* [?] what threshold? */

std::unique_ptr<IntValue> Machine::process(TokenStream &feature_stream) {
	auto tag_stream = generate_tags(feature_stream);
	collect_value_space(feature_stream);

	/* [TODO] apply filter */

	/* execute all qre leaves */
	for (int i=0; i<qre_list.size(); i++)
	{
		auto qre = qre_list[i];
		aggregate(qre, 0, feature_stream, tag_stream);
	}

	/* compute final result of num tree */
	return num_tree->eval();
}

/* lvl = agg stack level */
unique_ptr<IntValue> Machine::aggregate(shared_ptr<QRELeaf> qre, int lvl, TokenStream& feature_stream, vector<DT::Word>& tag_stream)
{
	/* stack top, execute transducer */
	if (lvl == qre->agg_stack.size())
	{
		vector< unique_ptr<DT::DataValue> > param;
		auto start = unique_ptr<StateValue>(new StateValue()); // starting symbol = true, stack size 1 (with dummy value)
		start->active->unknown = false;
		start->active->val = true;
		start->value_stack.push_back( unique_ptr<IntValue>(new IntValue(0)) );
		param.push_back(move(start));

		qre->transducer->reset(param);

		auto ans = qre->transducer->process(tag_stream);
		return copy_typed_data(IntValue, ((StateValue*)ans[0].get())->value_stack[0]);
	}

	/* otherwise recursion */
	/* split stream */
	map< vector<StreamFieldType>, pair<TokenStream, vector<DT::Word> > > sub_streams;
	for (int i=0; i<feature_stream.size(); i++)
	{
		vector<StreamFieldType> signature;
		vector<int> & p = qre->agg_stack[lvl].param;
		for (int j=0; j<p.size(); j++)
			signature.push_back(feature_stream[i][p[j]].value);
		sub_streams[signature].first.push_back(feature_stream[i]);
		sub_streams[signature].second.push_back(tag_stream[i]);
	}

	unique_ptr<IntValue> ans = nullptr;
	for_each(sub_streams.begin(), sub_streams.end(), [&](pair< vector<StreamFieldType>, pair<TokenStream, vector<DT::Word> > > cur) {
		auto candidate = aggregate(qre, lvl+1, cur.second.first, cur.second.second);
		if (ans == nullptr)
			ans = copy_typed_data(IntValue, candidate);
		else
			ans = qre->agg_stack[lvl].aggop->eval(ans.get(), candidate.get());
	});

	return ans;
}

void Machine::collect_value_space(TokenStream &stream) {
	int fields = stream[0].size();
	for (int i=0; i<fields; i++)
	{
		value_space.push_back(ValueSpace());
		for (int j=0; j<stream.size(); j++)
			value_space[i].range.insert(stream[j][i].value);
	}
}

vector<DT::Word> Machine::generate_tags(TokenStream &feature_stream)
{
	vector<DT::Word> tag_stream;
	for (int i=0; i<feature_stream.size(); i++)
	{
		DT::Word w;
//		w.val = DataValue::factory->get_instance(DT::UNDEF);
		for (int j=0; j<predicates.size(); j++)
		{
			auto pred_tag = unique_ptr<StateValue>(new StateValue());
			pred_tag->active = satisfy(predicates[j], feature_stream[i]);
			w.tag_bitmap.push_back(move(pred_tag));
		}
		tag_stream.push_back(w);
	}
	return tag_stream;
}

unique_ptr<BoolValue> Machine::satisfy(shared_ptr<NetqreAST> predicate, FeatureVector & fv)
{
	switch(predicate->bool_type)
	{
		case BoolOpType::OR:
		return OrOp::eval(satisfy(predicate->subtree[0], fv).get(), satisfy(predicate->subtree[1], fv).get());

		case BoolOpType::AND:
		return AndOp::eval(satisfy(predicate->subtree[0], fv).get(), satisfy(predicate->subtree[1], fv).get());

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

std::unique_ptr<IntValue> NumericalTree::eval()
{
	/*TODO*/
	return nullptr;
}

shared_ptr<Machine> Interpreter::interpret(std::shared_ptr<NetqreAST> ast) 
{
	auto machine = shared_ptr<Machine>(new Machine());
	collect_predicates(ast, machine->predicates);
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
	}

}

std::shared_ptr<NumericalTree> Interpreter::real_interpret_num(std::shared_ptr<NetqreAST> ast, std::shared_ptr<Machine> machine)
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
			case NumOpType::ADD:
			tree->op = shared_ptr<AddOp>(new AddOp());
			break;

			case NumOpType::SUB:
			tree->op = shared_ptr<SubOp>(new SubOp());
			break;

			case NumOpType::MUL:
			tree->op = shared_ptr<MulOp>(new MulOp());
			break;

			case NumOpType::DIV:
			tree->op = shared_ptr<DivOp>(new DivOp());
			break;
		}
		return tree;

		case NetqreExpType::QRE_VS:
		tree->is_leaf = true;
		tree->leaf = real_interpret_agg(ast, machine);
		machine->qre_list.push_back( tree->leaf );
		return tree;

		default:
		throw string("[real_interpret_num] Shouldn't reach here.");
	}
}

std::shared_ptr<QRELeaf> Interpreter::real_interpret_agg(std::shared_ptr<NetqreAST> ast, std::shared_ptr<Machine> machine)
{
	shared_ptr<NetqreAST> cur = ast;
	auto leaf = shared_ptr<QRELeaf>(new QRELeaf());
	while(true)
	{
		switch(cur->type)
		{
			case NetqreExpType::QRE_VS:
			{
				Aggregator agg;
				switch(cur->agg_type)
				{
					case AggOpType::MAX:
					agg.aggop = shared_ptr<MaxOp>(new MaxOp());
					break;

					case AggOpType::MIN:
					agg.aggop = shared_ptr<MinOp>(new MinOp());
					break;

					case AggOpType::SUM:
					agg.aggop = shared_ptr<AddOp>(new AddOp());
					break;

					case AggOpType::AVG:
					throw string("Not supported yet!\n");
				}
				auto feat = cur->subtree[1];
				for (int i = 0; i < feat->subtree.size(); i++)
				agg.param.push_back(feat->subtree[i]->value);
				leaf->agg_stack.push_back(agg);

				cur = cur->subtree[0];
				break;
			}

			case NetqreExpType::QRE_PS:
			leaf->transducer = real_interpret_qre(cur->subtree[0], machine);
			return leaf;
		}
	}
}

shared_ptr<DT::Transducer> Interpreter::real_interpret_qre(std::shared_ptr<NetqreAST> ast, std::shared_ptr<Machine> machine)
{
	auto parse_agg_commit_op = [](shared_ptr<NetqreAST> ast) -> shared_ptr<PopStackOp> {
		switch(ast->agg_type)
		{
			case AggOpType::MAX:
			return shared_ptr<PopStackOp>(new PopStackOp(shared_ptr<MaxOp>(new MaxOp())));

			case AggOpType::MIN:
			return shared_ptr<PopStackOp>(new PopStackOp(shared_ptr<MinOp>(new MinOp())));

			case AggOpType::SUM:
			return shared_ptr<PopStackOp>(new PopStackOp(shared_ptr<AddOp>(new AddOp())));

			case AggOpType::AVG:
			throw string("Not supported yet!\n");
		}
	};


	auto parse_agg_init_op = [](shared_ptr<NetqreAST> ast) -> shared_ptr<PushStackOp> {
		auto val = unique_ptr<IntValue>(new IntValue());
		switch(ast->agg_type)
		{
			case AggOpType::MAX:
			val->upper = 0;
			val->lower = 0;
			break;

			case AggOpType::MIN:
			val->upper = IntValue::MAXIMUM;
			val->lower = IntValue::MAXIMUM;
			break;

			case AggOpType::SUM:
			val->upper = 0;
			val->lower = 0;
			break;

			case AggOpType::AVG:
			throw string("Not supported yet!\n");
		}
		return shared_ptr<PushStackOp>(new PushStackOp(val));
	};

	switch(ast->type)
	{
		case NetqreExpType::QRE_PS:
		switch(ast->reg_type)
		{
			case RegularOpType::STAR:
			{
				auto agg_init_op = parse_agg_init_op(ast->subtree[1]);
				auto agg_commit_op = parse_agg_commit_op(ast->subtree[1]);
				auto dt_subexp = real_interpret_qre(ast->subtree[0],machine);
				dt_subexp->combine(nullptr, DT::CombineType::STAR, agg_init_op, agg_commit_op);
				return dt_subexp; 
			}

			case RegularOpType::CONCAT:
			{
				auto agg_init_op = parse_agg_init_op(ast->subtree[2]);
				auto agg_commit_op = parse_agg_commit_op(ast->subtree[2]);
				auto dt_left= real_interpret_qre(ast->subtree[0], machine);
				auto dt_right = real_interpret_qre(ast->subtree[1], machine);
				dt_left->combine(dt_right, DT::CombineType::CONCATENATION, agg_init_op, agg_commit_op);
				return dt_left;
			}
		}

		case NetqreExpType::QRE_COND:
		{
			auto dt_re = real_interpret_re(ast->subtree[0], machine);
			auto cond_op = shared_ptr<CondOp>(new CondOp());
			dt_re->combine(nullptr, DT::CombineType::CONDITIONAL, nullptr, cond_op);
			return dt_re;
		}
	}
	
}


shared_ptr<DT::Transducer> Interpreter::real_interpret_re(std::shared_ptr<NetqreAST> ast, shared_ptr<Machine> machine)
{
	switch(ast->type)
	{
		case NetqreExpType::RE:
		switch(ast->reg_type)
		{
			case RegularOpType::STAR:
			auto agg_init_op = shared_ptr<DT::CopyOp>(new DT::CopyOp());
			auto agg_commit_op = shared_ptr<TransitionOp>(new TransitionOp());
			auto dt_subexp = real_interpret_qre(ast->subtree[0], machine);
			dt_subexp->combine(nullptr, DT::CombineType::STAR, agg_init_op, agg_commit_op);
			return dt_subexp;

			case RegularOpType::CONCAT:
			auto agg_init_op = shared_ptr<DT::CopyOp>(new DT::CopyOp());
			auto agg_commit_op = shared_ptr<TransitionOp>(new TransitionOp());
			auto dt_left= real_interpret_qre(ast->subtree[0], machine);
			auto dt_right = real_interpret_qre(ast->subtree[1], machine);
			dt_letf->combine(dt_right, DT::CombineType::CONCATENATION, agg_init_op, agg_commit_op);
			return dt_left;
		}

		case NetqreExpType::WILDCARD:
		{
			auto dt = shared_ptr<Transducer>(new Transducer(machine->predicates.size()));
			shared_ptr<Circuit> bak;

			for (int i=0; i<machine->predicates.size(); i++)
			{
				auto c = shared_ptr<Circuit>(new Circuit());

				auto op = shared_ptr<DT::CopyOp>(new DT::CopyOp());

				auto gii = shared_ptr<Gate>(new Gate(op));
				auto gif = shared_ptr<Gate>(new Gate(op));
				auto goi = shared_ptr<Gate>(new Gate(op));
				auto gof = shared_ptr<Gate>(new Gate(op));
				gii->wire_out(gof);
				gof->wire_in(gii);

				c->add_gate(gii, STATE_IN_INIT);
				c->add_gate(gif, STATE_IN_FINAL);
				c->add_gate(goi, STATE_OUT_INIT);
				c->add_gate(gof, STATE_OUT_FINAL);

				bak = c;

				dt->add_circuit(c, i);
			}
			dt->add_epsilon_circuit(bak->get_plain_circuit());
		}

		case NetqreExpType::PREDICATE_SET:
		{
			auto dt = shared_ptr<Transducer>(new Transducer(machine->predicates.size()));
			auto c = shared_ptr<Circuit>(new Circuit());
			auto in_op = shared_ptr<DT::CopyOp>(new DT::CopyOp());
			auto out_op = shared_ptr<TransitionOp>(new TransitionOp());

			auto gs = shared_ptr<Gate>(new Gate(in_op));
			auto gii = shared_ptr<Gate>(new Gate(in_op));
			auto gif = shared_ptr<Gate>(new Gate(in_op));
			auto goi = shared_ptr<Gate>(new Gate(in_op));
			auto gof = shared_ptr<Gate>(new Gate(out_op));
			gii->wire_out(gof);
			gs->wire_out(gof);
			gof->wire_in(gii); // first value
			gof->wire_in(gs); // second predicate

			c->add_gate(gs, STREAM_IN);
			c->add_gate(gii, STATE_IN_INIT);
			c->add_gate(gif, STATE_IN_FINAL);
			c->add_gate(goi, STATE_OUT_INIT);
			c->add_gate(gof, STATE_OUT_FINAL);

			dt->add_circuit(c, ast->tag);
			dt->add_epsilon_circuit(c->get_plain_circuit());
		}
	}
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
		ast->tag = predicates.size();
		predicates.push_back(ast);
		return;
	}
}

}
