#include "interpreter.h"

using namespace std;

DT::Transducer NetqreInterpreter::interpret(std::shared_ptr<NetqreAST> ast) 
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
		case WILDCARD:
		case CONST:
		case THRESHOLD:
	}

}
