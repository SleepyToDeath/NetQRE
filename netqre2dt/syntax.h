#ifndef NETQRE2DT_SYNTAX_H
#define NETQRE2DT_SYNTAX_H

#include <memory>
#include "./network_tokenizer/feature_vector.hpp"
#include "../data-transducer/op.h"

/*
	<program>::=
		<filter> <qre> <threshold>
	
	<filter>::=
		filter( <predicate-set> )

	<predicate-set>::=
		&&(<predicate-set>,<predicate-set>)
		| ||(<predicate-set>,<predicate-set>
		| <predicate>
	
	<predicate>::=
		[<feature-ni> <pred-op> <value>]
		| [<unknown>]

	<unknown> = _
	
	<feature-ni>::= <ranged-int> (index in feature vector, must be non-iterative)
	<value>::= <ranged-int> (range of value)
	<pred-op>:= == | -> | <= | >=

	<qre>::=
		qre(<qre-ns>)

	(number space)
	<qre-ns>::= 
		<num-op>(<qre-ns>,<qre-ns>)
		| <qre-vs>
		| <const>
		# | <output>

	<num-op>::=
		+ | - | * | /
	
	(value space)
	<qre-vs>::=
		<agg_op> ( <qre-vs> , <feature-set> )
		| <qre-ps>
		# | <output>

	<agg_op>::=
		max | min | sum | avg

	<feature set>::=
		<feature set>, <feature-i>
		| <feature-i> 
		
	<feature-i>::= <ranged-int>	(index in feature vector, must be iterative )

	(position space)
	<qre-ps>::=
		split(<qre-ps>, <qre-ps>, <agg_op>)
		| iter(<qre-ps>, <agg_op>)
		| <qre-conditional>
		# | <output> 

	( limit possible output )
	<qre-cond>::=
		/<re>/?1
		# | <output>

	<output>::= <ranged-int>
	<const>::= <ranged-int>

	<re>::=
		*(<re>)
		| <re> <re>
		| <predicate-set>
		| <wildcard>

	<wildcard>::= _

	<threshold>::=
		threshold(<const>)
*/

namespace Netqre {


enum class NetqreExpType {
	PROGRAM, 		//0
	FILTER, 		//1
	PREDICATE_SET, 	//2
	PREDICATE, 		//3
	FEATURE_NI, 	//4
	VALUE, 			//5
	QRE,			//6
	QRE_NS, 		//7
	NUM_OP, 		//8
	QRE_VS, 		//9
	AGG_OP, 		//10
	FEATURE_SET,	//11
	FEATURE_I, 		//12
	QRE_PS,			//13
	QRE_COND,		//14
	RE,				//15
	RE_STAR,		//16
	OUTPUT,			//17
	WILDCARD,		//18
	CONST,			//19
	THRESHOLD,		//20
	UNKNOWN,		//21
	PENDING_LITERAL	//22
};

enum class AggOpType {
	MAX, MIN, SUM, AVG, NONE
};

enum class ArithOpType {
	ADD, SUB, MUL, DIV, NONE
};

enum class BoolOpType {
	AND, OR, NONE
};

enum class RegularOpType {
	STAR, CONCAT, NONE
};

enum class PredOpType {
	NONE, BIGGER, SMALLER, IN, EQUAL
};

class NetqreAST
{
	public:
	union {
		AggOpType agg_type;
		ArithOpType arith_type;
		BoolOpType bool_type;
		RegularOpType reg_type;
		StreamFieldType value;
		PredOpType pred_type;
	};

	NetqreAST() {};
	NetqreAST(NetqreExpType type) {this->type = type;}

	TagType tag;

	NetqreExpType type;

	Rubify::vector< std::shared_ptr<NetqreAST> > subtree;

	Rubify::string to_s()
	{
		return _S_((int)type) + "[ " + subtree.map<Rubify::string>([&](const std::shared_ptr<NetqreAST>& ptr )->Rubify::string {
			return ptr->to_s();
		}).to_s() + " ]";
	}
};
 
}

#endif
