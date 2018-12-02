#ifndef NETQRE2DT_INTERPRETER_H
#define NETQRE2DT_INTERPRETER_H

#include "syntax.h"
#include "../data-transducer/transducer.h"

class NetqreInterpreter
{
	public:
	DT::Transducer interpret(std::shared_ptr<NetqreAST> ast);
};

#endif
