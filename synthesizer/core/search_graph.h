#ifndef SEARCH_GRAPH_H
#define SEARCH_GRAPH_H

#include <unordered_set>
#include "../../general-lang/incomplete_execution.h"
#include "redundancy.h"

typedef std::unordered_set<shared_ptr<SyntaxTree>, HashSyntaxTree, CmpSyntaxTree > VisitPool;

class SearchGraph {
	public:

	/* incomplete execution version */
	SearchGraph(
				int answer_count, 
				shared_ptr<IESyntaxLeftHandSide> starting_symbol, 
				shared_ptr<RedundancyPlan> rp );
	vector<shared_ptr<IESyntaxTree> > search_top_level_v2(
		shared_ptr<IEExample> examples, 
		vector<shared_ptr<IESyntaxTree> > seed,
		VisitPool eliminate);

	static void enum_pure(
		vector<shared_ptr<IESyntaxTree> > seed,
		VisitPool eliminate);

	private:
	int search_depth;
	int batch_size;
	int explore_rate;
	double accuracy;
	int answer_count;
	int threads;
	int give_up_count;
	int VERBOSE_MODE;
	shared_ptr<SyntaxLeftHandSide> starting_symbol;
	shared_ptr<RedundancyPlan> rp;

	/* incomplete execution version */
	vector< shared_ptr<IESyntaxTree> > enumerate_random_v2(
		shared_ptr<IEExample> examples, 
		vector<shared_ptr<IESyntaxTree> > seed,
		VisitPool eliminate);
};

#endif
