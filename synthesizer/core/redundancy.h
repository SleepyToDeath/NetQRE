#ifndef REDUNDANCY_H
#define REDUNDANCY_H

#include "syntax_tree.h"


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
	std::vector<shared_ptr<UnconditionalRedundancyTemplate> > ucnd;
	std::vector<shared_ptr<ConditionalRedundancyTemplate> > cnd;
};



#endif
