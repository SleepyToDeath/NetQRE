#ifndef NETQRE2DT_INTERPRETER_H
#define NETQRE2DT_INTERPRETER_H

#include "op.hpp"
#include "../data-transducer/transducer.h"
#include <unordered_set>

namespace Netqre {

class ValueSpace;
class Aggregator;
class QRELeaf;
class NumericalTree;

/* the executable produced by interpreter */
class Machine {
	public:
	std::unique_ptr<IntValue> process(std::vector<DT::Word> &stream);

	/* constant, won't change after construction */
	std::vector< std::shared_ptr<QRELeaf> > qre_list;
	std::shared_ptr<NumericalTree> num_tree;
	std::shared_ptr<NetqreAST> filter;
	std::vector< std::shared_ptr<NetqreAST> > predicates;

	private:
	/* runtime info, initialize for each execution */
	std::vector<ValueSpace> value_space;
	void collect_value_space(std::vector<DT::Word> &stream);
	std::vector<DT::Word> generate_tags(TokenStream &feature_stream);
	std::unique_ptr<BoolValue> satisfy(shared_ptr<NetqreAST> predicate, const FeatureVector & fv);
};

/* top level, QRE_NS */
class NumericalTree {
	public:
	/*	
		Only evaluate numerical part. 
		All QRELeaves must already have output.
	*/
	std::unique_ptr<IntValue> eval();
	bool is_leaf;
	std::shared_ptr<QRELeaf> leaf;

	std::shared_ptr<MergeIntOp> op;
	std::shared_ptr<NumericalTree> left;
	std::shared_ptr<NumericalTree> right;
};

/* Each variable of QRE_NS is a QRE_VS, with a stack of aggregators over value space.
	At the top of the stack is a single transducer produced from QRE_PS. */
class QRELeaf {
	public:
	std::unique_ptr<IntValue> output;

	std::vector<Aggregator> agg_stack;
	std::shared_ptr<DT::Transducer> transducer;
};

class Aggregator {
	public:
	std::vector<int> param;
	std::shared_ptr<MergeIntOp> aggop;
};

class ValueSpace {
	public:
	StreamFieldType lower;
	StreamFieldType upper;
	std::unordered_set<StreamFieldType> range;
};

class Interpreter
{
	public:
	std::shared_ptr<Machine> interpret(std::shared_ptr<NetqreAST> ast);

	private:
	void 							real_interpret(std::shared_ptr<NetqreAST> ast, std::shared_ptr<Machine> machine);
	std::shared_ptr<NumericalTree> 	real_interpret_num(std::shared_ptr<NetqreAST> ast, std::shared_ptr<Machine> machine);
	std::shared_ptr<QRELeaf> 		real_interpret_agg(std::shared_ptr<NetqreAST> ast);
	std::shared_ptr<DT::Transducer> real_interpret_qre(std::shared_ptr<NetqreAST> ast);
	std::shared_ptr<DT::Transducer> real_interpret_re(std::shared_ptr<NetqreAST> ast)

	void collect_predicates(std::shared_ptr<NetqreAST> ast, std::vector<shared_ptr<NetqreAST> > & predicates);
};

}

#endif
