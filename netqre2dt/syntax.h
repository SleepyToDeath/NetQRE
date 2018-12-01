#ifndef NETQRE2DT_SYNTAX_H
#define NETQRE2DT_SYNTAX_H

#include <vector>
#include <memory>

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
		[<feature-ni> = <value>]
	
	<feature-ni>::= <ranged-int> (index in feature vector, must be non-iterative)
	<value>::= <ranged-int> (range of value)

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


enum NetqreExpType {
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
	OUTPUT,			//16
	WILDCARD,		//17
	CONST,			//18
	THRESHOLD		//19
};

enum AggOpType {
	MAX, MIN, SUM, AVG
};

enum NumOpType {
	ADD, SUB, MUL, DIV
};

enum BoolOpType {
	AND, OR
};

enum RegularOpType {
	STAR, CONCAT
};

class NetqreAST
{
	public:
	union {
		AggOpType agg_type;
		NumOpType num_type;
		BoolOpType bool_type;
		RegularOpType reg_type;
		int value;
	};

	NetqreExpType type;

	std::vector< std::shared_ptr<NetqreAST> > subtree;
};

#endif
