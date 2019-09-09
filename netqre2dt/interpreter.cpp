#include "interpreter.h"
#include "op.hpp"
#include <algorithm>
#include <utility>
#include <set>
#include "rpc/interface.hpp"

#include <iostream>
using std::cerr;
using std::endl;

using std::unique_ptr;
using std::shared_ptr;
using std::static_pointer_cast;
using std::move;
using std::for_each;
using std::pair;
using std::set;

namespace Netqre {

auto high_one_bit = [](StreamFieldType num)->StreamFieldType {
	StreamFieldType ans = (num == 0) ? 0 : 1;
	while (num > 1)
	{
		num = num >> 1;
		ans = ans << 1;
	}
	return ans;
};

/* [!] assume a leading guard bit in src */
auto align_n_floor = [](StreamFieldType src, StreamFieldType target)->StreamFieldType {
	StreamFieldType mask = high_one_bit(target) << 1;
	if (src < (target | mask)) 
	{
		while (!(src & mask))
			src = src << 1;
		return src ^ mask;
	}
	return src ^ high_one_bit(src);
};

/* [!] assume a leading guard bit in src */
auto align_n_celling = [](StreamFieldType src, StreamFieldType target)->StreamFieldType {
	StreamFieldType mask = high_one_bit(target) << 1;
	if (src < (target | mask))
	{
		StreamFieldType low_one_bit = 1;
		while (!(src & mask)) {
			src = src << 1;
			low_one_bit << 1;
		}
		src = (src ^ mask) + low_one_bit - 1 ;
	}
	return (src > target) ? target : src;
};


/* [TODO] modify parser, remove threshold */
/* [?] what threshold? */

std::unique_ptr<IntValue> Machine::process(TokenStream &feature_stream) {

	/* auto reset */
	reset();

	auto tag_stream = generate_tags(feature_stream);
	collect_value_space(feature_stream);

	/* [TODO] apply filter */

	/* execute all qre leaves */
	for (int i=0; i<qre_list.size(); i++)
	{
		auto qre = qre_list[i];
		qre->output = aggregate(qre, 0, feature_stream, tag_stream);
		if (qre->output->lower < 0)
			return copy_typed_data(IntValue, qre->output);
	}

	/* compute final result of num tree */
	return num_tree->eval();
}

bool check_predicate(int &last_feature, shared_ptr<NetqreAST> pred, vector< vector<StreamFieldType> >& range)
{
	if (pred->type == NetqreExpType::PREDICATE)
	{
		/* check whether is unknown */
		if (pred->subtree.size() == 1)
			return true;

		/* check features being listed in ascending order */
		auto l = pred->subtree[0];
		int index = l->value;
		if (index <= last_feature)
			return false;
		last_feature = index;

		/* check features being listed in ascending order */
		auto r = pred->subtree[1];
		StreamFieldType value = r->value;
		if (value < 0)
			return false;
		switch(pred->pred_type)
		{
			case PredOpType::EQUAL:
			case PredOpType::BIGGER:
			case PredOpType::SMALLER:
			{
				int bottom_index = align_n_floor(value, range[index].size()-1);
				if (bottom_index >= range[index].size())
					return false;
				break;
			}

			case PredOpType::IN:
			{
				StreamFieldType cap = align_n_celling(value, range[index][-1]);
				StreamFieldType bottom = align_n_floor(value, range[index][-1]);
				if (cap < range[index][0] || bottom > range[index][-1])
					return false;
				/* check if some value in range */
				int maybe = range[index].locate( [&](const StreamFieldType& e) { return e < bottom; } );
				if (maybe >= range[index].size() || range[index][maybe] > cap)
					return false;
				break;
			}
		}

	}
	else
	{
		for (int i=0; i<pred->subtree.size(); i++)
			if (!check_predicate(last_feature, pred->subtree[i], range))
				return false;
	}
	return true;
}

void Machine::bind_context(shared_ptr<NetqreExample> global_example)
{
	this->global_example = global_example;
}
	
bool Machine::valid()
{
	for (int i=0; i<qre_list.size(); i++)
	{
		auto cur = qre_list[i];
		int last_feat = -1;
		for (int j=0; j<cur->agg_stack.size(); j++)
			for (int k=0; k<cur->agg_stack[j].param.size(); k++)
			{
				int cur_feat = cur->agg_stack[j].param[k];
				if (cur_feat <= last_feat)
					return false;
				else
					last_feat = cur_feat;
			}
	}

	for (int i=0; i<predicates.size(); i++)
	{
		int last_feat = -1;
		if (!check_predicate(last_feat, predicates[i], global_example->range))
			return false;
	}
	return true;
}

void Machine::reset() {
	value_space.clear();
	for (int i=0; i<qre_list.size(); i++)
		qre_list[i]->output = nullptr;
}

/* lvl = agg stack level */
unique_ptr<IntValue> Machine::aggregate(shared_ptr<QRELeaf> qre, int lvl, TokenStream& feature_stream, vector<DT::Word>& tag_stream)
{
	/* stack top, execute transducer */
	if (lvl == qre->agg_stack.size())
	{
		vector< unique_ptr<DT::DataValue> > param;
		auto start = unique_ptr<StateValue>(new StateValue()); // starting symbol = true, stack size 1 (with dummy value)
		start->type = DT::VALID;
		start->active->unknown = false;
		start->active->val = true;
//		start->value_stack.push_back( unique_ptr<IntValue>(new IntValue(0)) );
		param.push_back(move(start));

		qre->transducer->reset(param);

		auto ans = qre->transducer->process(tag_stream);
		if (ans[0]->type == DT::VALID)
			return copy_typed_data(IntValue, ((StateValue*)(ans[0].get()))->value_stack[0]);
		else
			return unique_ptr<IntValue>(new IntValue(-1));
	}

	/* otherwise recursion */
	/* split stream */
	map< vector<StreamFieldType>, pair<TokenStream, vector<DT::Word> > > sub_streams;
	for (int i=0; i<feature_stream.size(); i++)
	{
		vector<StreamFieldType> signature;
		vector<int> & p = qre->agg_stack[lvl].param;
		for (int j=0; j<p.size(); j++)
			signature.push_back(feature_stream[i][p[j]]);
		sub_streams[signature].first.push_back(feature_stream[i]);
		sub_streams[signature].second.push_back(tag_stream[i]);
	}

	unique_ptr<IntValue> ans = nullptr;
	bool bad_exp_flag = false;
	for_each(sub_streams.begin(), sub_streams.end(), [&](pair< vector<StreamFieldType>, pair<TokenStream, vector<DT::Word> > > cur) {
		if (bad_exp_flag)
			return;
		auto candidate = aggregate(qre, lvl+1, cur.second.first, cur.second.second);
		if (candidate->lower < 0)
		{
			bad_exp_flag = true;
			ans = copy_typed_data(IntValue, candidate);
			return;
		}
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
			value_space[i].range.insert(stream[j][i]);
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
			StateValue* pred_tag = new StateValue();
			pred_tag->active = satisfy(predicates[j], feature_stream[i]);
#ifdef DT_DEBUG
			cerr<<"Satisfy:"<<pred_tag->to_string()<<endl;
#endif
			DT::DataValue* tmp = nullptr;
			/* [TODO] Not sure if it's compiler's bug or some bufferoverflow
				or I didn't make a thorough clean.
				Sometimes the value of tmp will be off by 8 from pred_tag */
			tmp = pred_tag;
//			*(unsigned long long*)(&tmp) = (unsigned long long) pred_tag;
			w.tag_bitmap.push_back(unique_ptr<DT::DataValue>(tmp));
		}
		tag_stream.push_back(w);
	}
	return tag_stream;
}


unique_ptr<BoolValue> Machine::satisfy(shared_ptr<NetqreAST> predicate, FeatureVector & fv)
{
	if (predicate->type == NetqreExpType::WILDCARD)
	{
		auto true_ans = unique_ptr<BoolValue> (new BoolValue());
		true_ans->unknown = false;
		true_ans->val = true;
		return true_ans;
	}

	switch(predicate->type)
	{
		case NetqreExpType::UNKNOWN:
		{
			auto unknown = unique_ptr<BoolValue> (new BoolValue());
			unknown->unknown = true;
			unknown->val = true;
			return unknown;
		}

		case NetqreExpType::PREDICATE:
		{
			if (predicate->subtree.size() == 1)
				return satisfy(predicate->subtree[0], fv);
			auto l = predicate->subtree[0];
			auto r = predicate->subtree[1];
			int index = l->value;
			StreamFieldType value = r->value;
			auto sat = unique_ptr<BoolValue> (new BoolValue());
			sat->unknown = false;
			vector< vector<StreamFieldType> >& range = global_example->range;
			switch(predicate->pred_type)
			{
				case PredOpType::EQUAL:
				{
					int cap_index = align_n_celling(value, range[index].size()-1);
					int bottom_index = align_n_floor(value, range[index].size()-1);
					StreamFieldType cap = range[index][cap_index];
					StreamFieldType bottom = range[index][cap_index];
					sat->val = (fv[index] >= bottom && fv[index] <= cap);
					break;
				}

				case PredOpType::BIGGER:
				{
					int bottom_index = align_n_floor(value, range[index].size()-1);
					StreamFieldType bottom = range[index][bottom_index];
					sat->val = (fv[index] >= bottom);
					break;
				}

				case PredOpType::SMALLER:
				{
					int cap_index = align_n_celling(value, range[index].size()-1);
					StreamFieldType cap = range[index][cap_index];
					sat->val = (fv[index] <= cap);
					break;
				}

				case PredOpType::IN:
				{
					StreamFieldType cap = align_n_celling(value, range[index][-1]);
					StreamFieldType bottom = align_n_floor(value, range[index][-1]);
					sat->val = (fv[index] >= bottom && fv[index] <= cap);
					break;
				}
			}
			return sat;
		}

		case NetqreExpType::PREDICATE_SET:
		switch(predicate->bool_type)
		{
			case BoolOpType::OR:
			return OrOp::eval(satisfy(predicate->subtree[0], fv).get(), satisfy(predicate->subtree[1], fv).get());

			case BoolOpType::AND:
			return AndOp::eval(satisfy(predicate->subtree[0], fv).get(), satisfy(predicate->subtree[1], fv).get());

			case BoolOpType::NONE:
			{
				return satisfy(predicate->subtree[0], fv);
			}
			
			default:
			throw string("Impossible predicate type!\n");
		}
	}
}

std::unique_ptr<IntValue> NumericalTree::eval()
{
	if (is_leaf)
		return copy_typed_data(IntValue, leaf->output);
	else
		return op->eval(left->eval().get(), right->eval().get());
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
#ifdef DT_DEBUG
	cerr<< "Top "<<(int)ast->type <<endl;
#endif
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

		case NetqreExpType::THRESHOLD:
		machine->threshold = ast->value;
		return;

		default:
		throw string("Impossible exp type!\n");
	}

}

std::shared_ptr<NumericalTree> Interpreter::real_interpret_num(std::shared_ptr<NetqreAST> ast, std::shared_ptr<Machine> machine)
{
#ifdef DT_DEBUG
	cerr<< "Num "<<(int)ast->type <<endl;
#endif
	auto tree = shared_ptr<NumericalTree>(new NumericalTree());
	switch(ast->type)
	{
		case NetqreExpType::QRE_NS:
		if (ast->subtree.size() == 1)
			return real_interpret_num(ast->subtree[0], machine);
		tree->is_leaf = false;
		tree->left = real_interpret_num(ast->subtree[0], machine);
		tree->right = real_interpret_num(ast->subtree[1], machine);
		switch(ast->arith_type)
		{
			case ArithOpType::ADD:
			tree->op = shared_ptr<AddOp>(new AddOp());
			break;

			case ArithOpType::SUB:
			tree->op = shared_ptr<SubOp>(new SubOp());
			break;

			case ArithOpType::MUL:
			tree->op = shared_ptr<MulOp>(new MulOp());
			break;

			case ArithOpType::DIV:
			tree->op = shared_ptr<DivOp>(new DivOp());
			break;

			default:
			throw string("Impossible num op type!\n");
		}
		return tree;

		case NetqreExpType::QRE_VS:
		tree->is_leaf = true;
		tree->leaf = real_interpret_agg(ast, machine);
		machine->qre_list.push_back( tree->leaf );
		return tree;

		default:
		throw string("[real_interpret_num] Shouldn't reach here.\n");
	}
}

std::shared_ptr<QRELeaf> Interpreter::real_interpret_agg(std::shared_ptr<NetqreAST> ast, std::shared_ptr<Machine> machine)
{
#ifdef DT_DEBUG
	cerr<< "Agg "<<(int)ast->type <<endl;
#endif
	shared_ptr<NetqreAST> cur = ast;
	auto leaf = shared_ptr<QRELeaf>(new QRELeaf());
	while(true)
	{
		switch(cur->type)
		{
			case NetqreExpType::QRE_VS:
			{
				Aggregator agg;
				bool flag = true;
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

					case AggOpType::NONE:
					flag = false;
					break;
				}
				if (flag)
				{
					auto feat = cur->subtree[1];
					for (int i = 0; i < feat->subtree.size(); i++)
					agg.param.push_back(feat->subtree[i]->value);
					leaf->agg_stack.push_back(agg);
				}

				cur = cur->subtree[0];
				break;
			}

			case NetqreExpType::QRE_PS:
			leaf->transducer = real_interpret_qre(cur, machine);
			return leaf;

			default:
			throw "Impossible agg exp type!\n";
		}
	}
}


shared_ptr<DT::Transducer> Interpreter::real_interpret_qre(std::shared_ptr<NetqreAST> ast, std::shared_ptr<Machine> machine)
{
#ifdef DT_DEBUG
	cerr<< "QRE "<<(int)ast->type <<endl;
#endif
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

			default:
			throw string("Impossible agg type!\n");
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
		{
#ifdef DT_DEBUG
			cerr<< "QRE type"<<(int)ast->reg_type <<endl;
#endif
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

				case RegularOpType::NONE:
				return real_interpret_qre(ast->subtree[0], machine);

				default:
				throw string("Impossible qre type!\n");
			}
		}

		case NetqreExpType::QRE_COND:
		{
			auto dt_re = real_interpret_re(ast->subtree[0], machine);
			auto cond_op = shared_ptr<CondOp>(new CondOp());
			dt_re->combine(nullptr, DT::CombineType::CONDITIONAL, nullptr, cond_op);
			return dt_re;
		}

		default:
		throw string("Impossible qre type!\n");
	}
	
}


shared_ptr<DT::Transducer> Interpreter::real_interpret_re(std::shared_ptr<NetqreAST> ast, shared_ptr<Machine> machine)
{
#ifdef DT_DEBUG
	cerr<< "RE "<<(int)ast->type <<endl;
#endif
	switch(ast->type)
	{
		case NetqreExpType::RE_STAR:
		case NetqreExpType::RE:
		switch(ast->reg_type)
		{
			case RegularOpType::STAR:
			{
				auto agg_init_op = shared_ptr<DT::CopyOp>(new DT::CopyOp());
				auto agg_commit_op = shared_ptr<DT::CopyOp>(new DT::CopyOp());
				auto dt_subexp = real_interpret_re(ast->subtree[0], machine);
				dt_subexp->combine(nullptr, DT::CombineType::STAR, agg_init_op, agg_commit_op);
				return dt_subexp;
			}

			case RegularOpType::CONCAT:
			{
				auto agg_init_op = shared_ptr<DT::CopyOp>(new DT::CopyOp());
				auto agg_commit_op = shared_ptr<DT::CopyOp>(new DT::CopyOp());
				auto dt_left= real_interpret_re(ast->subtree[0], machine);
				auto dt_right = real_interpret_re(ast->subtree[1], machine);
				dt_left->combine(dt_right, DT::CombineType::CONCATENATION, agg_init_op, agg_commit_op);
				return dt_left;
			}

			case RegularOpType::NONE:
			{
				return real_interpret_re(ast->subtree[0], machine);
			}

			default:
			throw string("Impossible reg op type!\n");
		}

		/*
		case NetqreExpType::WILDCARD:
		{
			auto trans_op = shared_ptr<TransitionOp>(new TransitionOp());
			auto dt = shared_ptr<DT::Transducer>(new DT::Transducer(machine->predicates.size(), trans_op));
			shared_ptr<DT::Circuit> bak;

			for (int i=0; i<machine->predicates.size(); i++)
			{
				auto c = shared_ptr<DT::Circuit>(new DT::Circuit());

				auto op = shared_ptr<DT::CopyOp>(new DT::CopyOp());

				auto gii = shared_ptr<DT::Gate>(new DT::Gate(op));
				auto gif = shared_ptr<DT::Gate>(new DT::Gate(op));
				auto goi = shared_ptr<DT::Gate>(new DT::Gate(op));
				auto gof = shared_ptr<DT::Gate>(new DT::Gate(op));
				gii->wire_out(gof);
				gof->wire_in(gii);

				c->add_gate(gii, DT::GateType::STATE_IN_INIT);
				c->add_gate(gif, DT::GateType::STATE_IN_FINAL);
				c->add_gate(goi, DT::GateType::STATE_OUT_INIT);
				c->add_gate(gof, DT::GateType::STATE_OUT_FINAL);

				bak = c;

				dt->add_circuit(c, i);
			}
			dt->add_epsilon_circuit(bak->get_plain_circuit());
			return dt;
		}
		*/

		/* see wildcard as a predicate */
		case NetqreExpType::WILDCARD:
		case NetqreExpType::PREDICATE_SET:
		{
			auto trans_op = shared_ptr<TransitionOp>(new TransitionOp());
			auto dt = shared_ptr<DT::Transducer>(new DT::Transducer(machine->predicates.size(), trans_op));
			auto c = shared_ptr<DT::Circuit>(new DT::Circuit());
			auto in_op = shared_ptr<DT::CopyOp>(new DT::CopyOp());
			auto out_op = shared_ptr<PredicateOp>(new PredicateOp());

			auto gs = shared_ptr<DT::Gate>(new DT::Gate(in_op, "ps"));
			auto gii = shared_ptr<DT::Gate>(new DT::Gate(in_op, "pii"));
			auto gif = shared_ptr<DT::Gate>(new DT::Gate(in_op, "pif"));
			auto goi = shared_ptr<DT::Gate>(new DT::Gate(in_op, "poi"));
			auto gof = shared_ptr<DT::Gate>(new DT::Gate(out_op, "pof"));
			gii->wire_out(gof);
			gs->wire_out(gof);
			gof->wire_in(gii); // first value
			gof->wire_in(gs); // second predicate

			c->add_gate(gs, DT::GateType::STREAM_IN);
			c->add_gate(gii, DT::GateType::STATE_IN_INIT);
			c->add_gate(gif, DT::GateType::STATE_IN_FINAL);
			c->add_gate(goi, DT::GateType::STATE_OUT_INIT);
			c->add_gate(gof, DT::GateType::STATE_OUT_FINAL);

/*
			auto e = shared_ptr<DT::Circuit>(new DT::Circuit());
			auto _gs = shared_ptr<DT::Gate>(new DT::Gate(in_op, "ps"));
			auto _gii = shared_ptr<DT::Gate>(new DT::Gate(in_op, "pii"));
			auto _gif = shared_ptr<DT::Gate>(new DT::Gate(in_op, "pif"));
			auto _goi = shared_ptr<DT::Gate>(new DT::Gate(in_op, "poi"));
			auto _gof = shared_ptr<DT::Gate>(new DT::Gate(in_op, "pof"));
			e->add_gate(_gs, DT::GateType::STREAM_IN);
			e->add_gate(_gii, DT::GateType::STATE_IN_INIT);
			e->add_gate(_gif, DT::GateType::STATE_IN_FINAL);
			e->add_gate(_goi, DT::GateType::STATE_OUT_INIT);
			e->add_gate(_gof, DT::GateType::STATE_OUT_FINAL);
			*/
			auto e = c->get_plain_circuit();

			if (c->size() != e->size())
				throw string("Incorrect circuit construction!\n");

			dt->add_circuit(c, ast->tag);
			dt->add_epsilon_circuit(e);
			return dt;
		}

		default:
		throw string("Impossible re type!\n");
	}
}

void Interpreter::collect_predicates(std::shared_ptr<NetqreAST> ast, vector<shared_ptr<NetqreAST> > & predicates)
{
	/* [TODO] remove when filter is supported */
	if (ast->type == NetqreExpType::FILTER)
		return;
	if (ast->type != NetqreExpType::PREDICATE_SET && ast->type != NetqreExpType::WILDCARD)
	{
		for (int i=0; i<ast->subtree.size(); i++)
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
