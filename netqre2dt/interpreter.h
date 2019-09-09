#ifndef NETQRE2DT_INTERPRETER_H
#define NETQRE2DT_INTERPRETER_H

#include "op.hpp"
#include "../data-transducer/transducer.h"
#include <unordered_set>

class NetqreExample;

namespace Netqre {

class ValueSpace;
class Aggregator;
class QRELeaf;
class NumericalTree;

/* the executable produced by interpreter */
class Machine {
	public:
	/* can run multiple times, auto reset each time */
	std::unique_ptr<IntValue> process(TokenStream &feature_stream);

	bool valid();
	void bind_context(shared_ptr<NetqreExample> global_example);

	/* constant, won't change after construction */
	vector< std::shared_ptr<QRELeaf> > qre_list;
	std::shared_ptr<NumericalTree> num_tree;
	std::shared_ptr<NetqreAST> filter;
	vector< std::shared_ptr<NetqreAST> > predicates;
	StreamFieldType threshold;

	private:
	/* runtime info, initialize for each execution */
	vector<ValueSpace> value_space;
	std::shared_ptr<NetqreExample> global_example;

	void reset();
	void collect_value_space(TokenStream &stream);

	unique_ptr<IntValue> aggregate(shared_ptr<QRELeaf> qre, int lvl, TokenStream& feature_stream, vector<DT::Word>& tag_stream);
	vector<DT::Word> generate_tags(TokenStream &feature_stream);
	std::unique_ptr<BoolValue> satisfy(shared_ptr<NetqreAST> predicate, FeatureVector & fv);

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

	vector<Aggregator> agg_stack;
	std::shared_ptr<DT::Transducer> transducer;
};

class Aggregator {
	public:
	vector<int> param;
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
	std::shared_ptr<QRELeaf> 		real_interpret_agg(std::shared_ptr<NetqreAST> ast, std::shared_ptr<Machine> machine);
	std::shared_ptr<DT::Transducer> real_interpret_qre(std::shared_ptr<NetqreAST> ast, std::shared_ptr<Machine> machine);
	std::shared_ptr<DT::Transducer> real_interpret_re(std::shared_ptr<NetqreAST> ast, shared_ptr<Machine> machine);

	void collect_predicates(std::shared_ptr<NetqreAST> ast, vector<shared_ptr<NetqreAST> > & predicates);
};

}

#endif
