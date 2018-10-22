#include "redundancy.h"
#include <iostream>
using namespace std;

shared_ptr<IESyntaxTree> RedundancyPlan::filter(shared_ptr<IESyntaxTree> suspect, shared_ptr<IEExample> examples) {
	/* check conditional */
	for (int i=0; i<cnd.size(); i++)
	{
//		cout<<"[Begin matching]"<<suspect->to_string()<<endl;
		/* prepare */
		auto r = cnd[i];
//		cout<<"Template src:"<<r->temp->to_string()<<endl;
		vector<shared_ptr<IESyntaxTree> > checklist;
		bool match_flag = true;
		for (int j=0; j<r->checklist.size(); j++)
		{
//			cout<<"Template dst:"<<r->checklist[j]->to_string()<<endl;
			auto entry = suspect->search_and_replace(r->temp, r->checklist[j]);
			if (entry == nullptr)
			{
				match_flag = false;
//				cout<<"[No matching]#"<<i<<endl;
				break;
			}
			checklist.push_back(std::static_pointer_cast<IESyntaxTree>(entry));
		}
		if (!match_flag)
			continue;
			/*
		cout<<"[Matching]#"<<i<<endl;
		{
			for (int i=0; i<checklist.size(); i++)
				cout<<checklist[i]->to_string()<<" | ";
			cout<<endl;
		}
		*/

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
//			cout<<"[Rejected]\n\n";
			return nullptr;
		}
//		cout<<"[Accepted]\n\n";
	}

	/* check unconditional */
	for (int i=0; i<ucnd.size(); i++)
	{
		auto r = ucnd[i];
		auto maybe = suspect->search_and_replace(r->temp_src, r->temp_dst);
		if (maybe != nullptr)
			return std::static_pointer_cast<IESyntaxTree>(maybe);
	}

	/* no redundancy */
	return suspect;
}
