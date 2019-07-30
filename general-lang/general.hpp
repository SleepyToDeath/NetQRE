#ifndef GENERAL_LANG_GENERAL_HPP
#define GENERAL_LANG_GENERAL_HPP

#include "incomplete_execution.h"

/*
	This file adds preipheral interfaces to syntax tree and program types.
	They mainly adds usability, allowing operations such as program parsing 
	and testing, grammar generation, syntax tree pruning, etc.
*/

using Rubify::string;
using Rubify::vector;
using Rubify::map;
using std::static_pointer_cast;

class GeneralExample;
class GeneralSyntaxRightHandSide;
class GeneralSyntaxTree;

/*
class GeneralMatchingResult {
	public:
	bool accept;
	double utility_rate;
};
*/

typedef bool GeneralMatchingResult;

class GeneralTestResult {
	public:
	double pos_accuracy;
	double neg_accuracy;
};

class AbstractCode {
	public:
	AbstractCode(std::string _pos, std::string _neg):pos(_pos),neg(_neg),completable(true){}
	AbstractCode():pos(""),neg(""),completable(false){}
	std::string pos;
	std::string neg;
	bool completable;
};

class GeneralInterpreter {
	public:
	virtual GeneralMatchingResult accept(AbstractCode code, bool complete,  shared_ptr<GeneralExample> input, IEConfig cfg = IEConfig()) { return !complete; }
	virtual GeneralMatchingResult accept(shared_ptr<GeneralSyntaxTree> code, bool complete,  shared_ptr<GeneralExample> input, IEConfig cfg = IEConfig()) { return !complete; }
	virtual double extra_complexity(AbstractCode code) { return 0.0; }
	virtual double extra_complexity(shared_ptr<GeneralSyntaxTree> code) { return 0.0; }
	virtual vector<string> get_range(int handle, shared_ptr<GeneralExample> input) { return vector<string>(); }
	virtual GeneralTestResult test(string code, shared_ptr<GeneralExample> input) 
		{ GeneralTestResult res; res.pos_accuracy = 0; res.neg_accuracy = 0; return res; }
};


class GeneralExampleHandle;

class GeneralExample: public IEExample {
	public:
	vector<string> positive;
	vector<string> negative;
	virtual shared_ptr<GeneralExampleHandle> to_handle(int pos_offset = 0, int neg_offset = 0) = 0;
};

/* used to save space/bandwidth when you 
	don't really need the content of the example */
class GeneralExampleHandle: public GeneralExample{
	public:
	vector<int> positive_token;
	vector<int> negative_token;

	bool informative; /* used for synthesis */

	virtual shared_ptr<GeneralExampleHandle> to_handle(int pos_offset = 0, int neg_offset = 0) {
		return static_pointer_cast<GeneralExampleHandle>(shared_from_this());
	}
};



/* somehow build source code when initializing
 * accept() will feed the source code and input to external interpreter */
class GeneralProgram: public IEProgram {
	public:
	GeneralProgram(AbstractCode _src, bool _complete):
	source_code(_src), complete(_complete) {
	}

	bool accept( shared_ptr<IEExample> input, IEConfig cfg = DEFAULT_IE_CONFIG) {
		if (!source_code.completable)
			return true;
//		else
//			cerr<<"Completable Found!"<<endl;
		return interpreter->accept(source_code, complete, std::static_pointer_cast<GeneralExample>(input), cfg);
//		utility_rate = r.utility_rate;
//		return r.accept;
	}

	/* set this to the interpreter of your specific language */
	static std::unique_ptr<GeneralInterpreter> interpreter;

	bool complete;
//	double utility_rate = 0.0000000000001;
	AbstractCode source_code;
};

class GeneralSyntaxLeftHandSide;

class GeneralSyntaxRightHandSide : public IESyntaxRightHandSide {
	public:
	GeneralSyntaxRightHandSide() { name = ""; }
	std::string to_string(vector<std::string> subs) { return ""; }
	shared_ptr<IEProgram> combine_subprograms(vector< shared_ptr<IEProgram> > subprograms) {return nullptr;}

	vector<std::shared_ptr<GeneralSyntaxLeftHandSide> > subexp_full;
};

class GeneralSyntaxLeftHandSide : public IESyntaxLeftHandSide {

	public:

	/* dummy initializer */
	GeneralSyntaxLeftHandSide() {
		is_term = true;
	}

	bool is_functional() { 
		return functional;
	}

	std::string positive_abstract_code = "";
	std::string negative_abstract_code = "";
	bool functional; /* example of non-functional symbol : "(", ")", ",", non-functional symbols start with "$" */

	private:

	shared_ptr<IEProgram> to_program() {return nullptr;}
};

class GeneralSyntaxTree : public IESyntaxTree {
	public:

	shared_ptr<GeneralProgram> program = nullptr;
	int prune_count;

	GeneralSyntaxTree(shared_ptr<SyntaxTree> src):	IESyntaxTree(src) { prune_count = static_pointer_cast<GeneralSyntaxTree>(src)->prune_count;}
	GeneralSyntaxTree(shared_ptr<SyntaxTreeNode> root, int depth, int prune_count): IESyntaxTree(root, depth) { this->prune_count = prune_count;}
	shared_ptr<IEProgram> to_program() {
		if (program == nullptr)
			program = shared_ptr<GeneralProgram>(new GeneralProgram(to_code(), is_complete()));
		return program;
	}

	virtual double get_complexity() {
		if (complexity == 0)
			complexity = GeneralProgram::interpreter->extra_complexity(static_pointer_cast<GeneralSyntaxTree>(shared_from_this()));
		return complexity;
	}


	AbstractCode to_code() {
		std::string pos;
		std::string neg;
		if (root->get_type()->is_term) 
		{
			pos = root->get_type()->name;
			neg = pos;
		}
		else if (root->get_option() == SyntaxLeftHandSide::NoOption)
		{
			pos = std::static_pointer_cast<GeneralSyntaxLeftHandSide>(root->get_type())->positive_abstract_code;
			neg = std::static_pointer_cast<GeneralSyntaxLeftHandSide>(root->get_type())->negative_abstract_code;
			/* [!] it is assumed that pos & neg are either both completable or both uncompletable */
			if (pos == "")
				return AbstractCode();
		}
		else
		{
			auto rhs = std::static_pointer_cast<GeneralSyntaxRightHandSide> (root->get_type()->option[root->get_option()]);
			int j = 0;
			bool completable_flag = true;
			for (int i=0; i<rhs->subexp_full.size(); i++)	
			{
				if (rhs->subexp_full[i]->is_functional()) 
				{
					auto sub = (std::static_pointer_cast<GeneralSyntaxTree>(subtree[j])->to_code());
					if (!sub.completable)
					{
						completable_flag = false;
						break;
					}
					pos = pos + sub.pos;
					neg = neg + sub.neg;
					j++;
				}
				else {
					pos = pos + (rhs->subexp_full[i]->name);
					neg = neg + (rhs->subexp_full[i]->name);
				}
			}

			if (!completable_flag)
			{
				pos = std::static_pointer_cast<GeneralSyntaxLeftHandSide>(root->get_type())->positive_abstract_code;
				neg = std::static_pointer_cast<GeneralSyntaxLeftHandSide>(root->get_type())->negative_abstract_code;
				/* [!] it is assumed that pos & neg are either both completable or both uncompletable */
				if (pos == "")
					return AbstractCode();
			}
		}
		return AbstractCode(pos, neg);
	}


	std::string to_string() {
		std::string s;
		if (root->get_type()->is_term) 
			s = root->get_type()->name;
		else if (root->get_option() == SyntaxLeftHandSide::NoOption)
			s = std::static_pointer_cast<GeneralSyntaxLeftHandSide>(root->get_type())->name;
		else
		{
			auto rhs = std::static_pointer_cast<GeneralSyntaxRightHandSide> (root->get_type()->option[root->get_option()]);
			int j = 0;
			for (int i=0; i<rhs->subexp_full.size(); i++)	{
				if (rhs->subexp_full[i]->is_functional()) {
					s = s + (subtree[j]->to_string());
					j++;
				}
				else {
					s = s + (rhs->subexp_full[i]->name);
				}
			}
		}
		return s;
	}

	void copy_initializer(shared_ptr<SyntaxTree> src) {
		for (int i=0; i<src->subtree.size(); i++)
			subtree.push_back(shared_ptr<GeneralSyntaxTree>(new GeneralSyntaxTree(src->subtree[i])));
	}

	void prune(int prune_depth)
	{
		prune_count = 1;
		if (get_depth() <= prune_depth)
			return;
		for (int i=0; i<subtree.size(); i++)
		{
			auto sub = static_pointer_cast<GeneralSyntaxTree>(subtree[i]);
			if (sub->get_depth() == prune_depth)
			{
				vector<shared_ptr<SyntaxTree> >& shortcut = sub->root->get_type()->shortcut;
				bool exist = false;
				for (int j=0; j<shortcut.size(); j++)
				{
					if (sub->equal(shortcut[j]))
					{
						exist = true;
						break;
					}
				}
				if (!exist)
				{
					shortcut.push_back(SyntaxTree::factory->get_new(sub));
				}
				sub->harvest();
				sub->root->set_option(SyntaxLeftHandSide::NoOption);
				sub->subtree.clear();
			}
			else if (sub->get_depth() > prune_depth)
			{
				sub->prune(prune_depth);
			}
		}
	}

	void harvest()
	{
		if (get_depth() <= 2)
			return;
		for (int i=0; i<subtree.size(); i++)
		{
			auto sub = static_pointer_cast<GeneralSyntaxTree>(subtree[i]);
			sub->harvest();
			vector<shared_ptr<SyntaxTree> >& shortcut = sub->root->get_type()->shortcut;
			bool exist = false;
			for (int j=0; j<shortcut.size(); j++)
			{
				if (sub->equal(shortcut[j]))
				{
					exist = true;
					break;
				}
			}
			if (!exist)
			{
				shortcut.push_back(SyntaxTree::factory->get_new(sub));
			}
		}
	}

	int get_depth()
	{
		if (root->get_type()->is_term)
			return 1;
		else
		{
			int d = 0;
			for (int i=0; i<subtree.size(); i++)
			{
				int d_sub = static_pointer_cast<GeneralSyntaxTree>(subtree[i])->get_depth();
				if (d_sub > d)
					d = d_sub;
			}
			d++;
			return d;
		}
	}
};

class GeneralSyntaxTreeFactory : public SyntaxTreeFactory {
	public:

	shared_ptr<SyntaxTree> get_new( shared_ptr<SyntaxTreeNode> root, int depth) 
	{
		return shared_ptr<GeneralSyntaxTree>(new GeneralSyntaxTree(root, depth, 0));
	}

	shared_ptr<SyntaxTree> get_new( shared_ptr<SyntaxTreeNode> root) 
	{
		return shared_ptr<GeneralSyntaxTree>(new GeneralSyntaxTree(root, 0, 0));
	}

	shared_ptr<SyntaxTree> get_new( shared_ptr<SyntaxTree> src) 
	{
		return shared_ptr<GeneralSyntaxTree>(new GeneralSyntaxTree(src));
	}
};

std::unique_ptr<SyntaxTreeFactory> SyntaxTree::factory = unique_ptr<GeneralSyntaxTreeFactory>(new GeneralSyntaxTreeFactory());

#endif
