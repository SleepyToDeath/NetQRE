#include "redundancy.h"
#include <iostream>

shared_ptr<IESyntaxTree> RedundancyPlan::filter(shared_ptr<IESyntaxTree> suspect, shared_ptr<IEExample> examples) {
	/* check conditional */
	for (int i=0; i<cnd.size(); i++)
	{
		/* prepare */
		auto r = cnd[i];
		vector<shared_ptr<IESyntaxTree> > checklist;
		bool match_flag = true;
		for (int j=0; j<r->checklist.size(); j++)
		{
			auto entry = suspect->search_and_replace(r->temp, r->checklist[j]);
			if (entry == nullptr)
			{
				match_flag = false;
				break;
			}
			checklist.push_back(std::static_pointer_cast<IESyntaxTree>(entry));
		}
		if (!match_flag)
			continue;

		/* set all/exist program flag */
		bool program_flag;
		if (r->all_program)
			program_flag = true;
		else
			program_flag = false;

		/* set all/exist acc/rej example flag */
		IEConfig cfg;
		cfg.pos_check = true;
		cfg.pos_accept = r->accept;
		cfg.pos_all = r->all_example;
		cfg.neg_check = false;

		/* check */
		for (int j=0; j<checklist.size(); j++)
		{
//			cout<<"[Checking Redundancy] "<<checklist[j]->to_string()<<endl;
			bool example_flag = checklist[j]->to_program()->accept(examples, cfg);
			if (!example_flag && r->all_program)
			{
				program_flag = false;
				break;
			}
			if (example_flag && !r->all_program)
			{
				program_flag = true;
				break;
			}
		}

		/* summary check */
		if (program_flag)
		{
			return nullptr;
		}
	}

	/* check unconditional */
	while (true)
	{
		shared_ptr<SyntaxTree> maybe = suspect;
		for (int i=0; i<ucnd.size(); i++)
		{
			auto r = ucnd[i];
			auto tmp = maybe->search_and_replace(r->temp_src, r->temp_dst);
			if (tmp != nullptr)
			{
				maybe = tmp;
				break;
			}
		}
		if (maybe != suspect)
			return nullptr;
//			return std::static_pointer_cast<IESyntaxTree>(maybe);
		else
			break;
	}

	/* no redundancy */
	return suspect;
}
