#ifndef NETQRE2DT_INTERPRETER_H
#define NETQRE2DT_INTERPRETER_H

#include "syntax.h"
#include "../data-transducer/transducer.h"

namespace Netqre {

class Interpreter
{
	public:
	DT::Transducer interpret(std::shared_ptr<NetqreAST> ast);

	private:
	void collect_predicates(std::shared_ptr<NetqreAST> ast, std::vector<shared_ptr<NetqreAST> > & predicates);
};

}

#endif
