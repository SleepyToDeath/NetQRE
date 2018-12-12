#ifndef NETQRE2DT_INTERPRETER_H
#define NETQRE2DT_INTERPRETER_H

#include "op.hpp"
#include "syntax.h"
#include "../data-transducer/transducer.h"

namespace Netqre {

class Aggregator;
class QRELeaf;
class NumericalTree;

class Machine {
	public:
	std::unique_ptr<IntValue> process(std::vector<DT::Word> stream);
	
	std::vector< shared_ptr<QRELeaf> > qre_list;
	std::shared_ptr<NumberTree> num_tree;
	std::shared_ptr<NetqreAST> filter;
};

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

class Interpreter
{
	public:
	std::shared_ptr<Machine> interpret(std::shared_ptr<NetqreAST> ast);
	std::vector<DT::Word> Interpreter::generate_tags(std::vector<shared_ptr<NetqreAST> > & predicates, TokenStream feature_stream);

	private:
	void real_interpret(std::shared_ptr<NetqreAST> ast, std::shared_ptr<Machine> machine);
	std::shared_ptr<NumericalTree> real_interpret_num(std::shared_ptr<NetqreAST> ast, std::shared_ptr<Machine> machine);
	std::shared_ptr<QRELeaf> real_interpret_agg(std::shared_ptr<NetqreAST> ast);
	std::shared_ptr<DT::Transducer> real_interpret_qre(std::shared_ptr<NetqreAST> ast);
	void collect_predicates(std::shared_ptr<NetqreAST> ast, std::vector<shared_ptr<NetqreAST> > & predicates);
	std::unique_ptr<BoolValue> Interpreter::satisfy(shared_ptr<NetqreAST> predicate, const FeatureVector & fv);
};

}

#endif
