#ifndef REDUNDANCY_H
#define REDUNDANCY_H

#include "../../general-lang/incomplete_execution.h"
#include <unordered_set>

class UnconditionalRedundancyTemplate {
	public:
	shared_ptr<SyntaxTreeTemplate> temp_src;
	shared_ptr<SyntaxTreeTemplate> temp_dst;
};

class ConditionalRedundancyTemplate {
	public:
	shared_ptr<SyntaxTreeTemplate> temp;
	vector<shared_ptr<SyntaxTreeTemplate> > checklist;
	bool all_example;
	bool all_program;
	bool accept;
};

class RedundancyPlan {
	public:
	vector<shared_ptr<UnconditionalRedundancyTemplate> > ucnd;
	vector<shared_ptr<ConditionalRedundancyTemplate> > cnd;
	shared_ptr<std::unordered_set<shared_ptr<SyntaxTree>, HashSyntaxTree, CmpSyntaxTree > > visited;
	/* If meet any condition, return null, which means drop it
		Otherwise, return the original/simplified program */
	/* Will first check all conditional redundancy.
		Then will apply the first matching unconditional simplification */
	shared_ptr<IESyntaxTree> filter(shared_ptr<IESyntaxTree> suspect, shared_ptr<IEExample> examples); 
};



#endif
