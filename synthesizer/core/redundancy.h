#ifndef INCOMPLETE_EXECUTION_H
#define INCOMPLETE_EXECUTION_H

#include "syntax_tree.h"


class UnconditionalRedundancyTemplate {
	public:
	shared_ptr<SyntaxTreeTemplate> temp_src;
	shared_ptr<SyntaxTreeTemplate> temp_dst;
};

class ConditionalRedundancyTemplate {
	public:
	shared_ptr<SyntaxTreeTemplate> temp;
	bool all_example;
	bool all_program;
	bool accept;
};

class RedundancyPlan {
	public:
	std::vector<shared_ptr<UnconditionalRedundancyTemplate> > ucnd;
	std::vector<shared_ptr<ConditionalRedundancyTemplate> > cnd;
};



#endif
