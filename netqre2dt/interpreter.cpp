#include "interpreter.h"
#include "op.hpp"

using std::unique_ptr;
using std::shared_ptr;
using std::string;
using std::vector;
using std::static_pointer_cast;
using std::move;

namespace Netqre {

DT::Transducer Interpreter::interpret(std::shared_ptr<NetqreAST> ast) 
{
	switch(ast->type)
	{
		case PROGRAM:
		case FILTER:
		case PREDICATE_SET:
		case PREDICAT:
		case FEATURE_NI:
		case VALUE:
		case QRE:
		case QRE_NS:
		case NUM_OP:
		case QRE_VS:
		case AGG_OP, 		
		case FEATURE_SET:
		case FEATURE_I:
		case QRE_P:
		case QRE_COND:
		case RE:
		case OUTPUT:
		case CONST:
		case THRESHOLD:
		case WILDCARD:
		case UNKNOWN:
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
		case DT::BoolOpType::OR
		return OrOp::eval(satisfy(predicate->subtree[0], fv), satisfy(predicate->subtree[1], fv));

		case DT::BoolOpType::AND:
		return AndOp::eval(satisfy(predicate->subtree[0], fv), satisfy(predicate->subtree[1], fv));

		case DT::BoolOpType::NONE:
		
	}
		
}

}
