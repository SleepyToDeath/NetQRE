#ifndef NETQRE2DT_INTERPRETER_H
#define NETQRE2DT_INTERPRETER_H

#include "syntax.h"
#include "../data-transducer/transducer.h"

namespace Netqre {

class Interpreter
{
	public:
	DT::Transducer interpret(std::shared_ptr<NetqreAST> ast);
	vector<DT::Word> Interpreter::generate_tags(std::vector<shared_ptr<NetqreAST> > & predicates, TokenStream feature_stream);

	private:
	void collect_predicates(std::shared_ptr<NetqreAST> ast, std::vector<shared_ptr<NetqreAST> > & predicates);
	unique_ptr<BoolValue> Interpreter::satisfy(shared_ptr<NetqreAST> predicate, const FeatureVector & fv);
};

}

#endif
