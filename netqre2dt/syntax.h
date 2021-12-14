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
	QRE,			//2
	QRE_NS, 		//3
	NUM_OP, 		//4
	QRE_VS, 		//5
	AGG_OP, 		//6
	FEATURE_SET,	//7
	FEATURE_I, 		//8
	QRE_PS,			//9
	QRE_COND,		//10
	RE,				//11
	RE_STAR,		//12
	PREDICATE_SET, 	//13
	PREDICATE, 		//14
	FEATURE_NI, 	//15
	WILDCARD,		//16
	CONST,			//17
	THRESHOLD,		//18
	UNKNOWN,		//19
	VALUE_BIN,		//20
	VALUE_DEC,		//21
	VALUE_DIGIT,	//22
	VALUE_SET,		//23
	PENDING_LITERAL	//24
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

	Rubify::string name;

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
