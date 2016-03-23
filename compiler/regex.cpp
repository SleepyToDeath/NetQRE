#include <iostream>
#include <sstream>
#include <stdio.h>
#include <map>
#include <typeinfo>
#include "declarations.h"
#include "util.h"
//#include "packet.h"

using namespace std;

// RE
extern map<RE*, int> re_id_map;
// This is for parser.cpp
//void RE::emit(ostream& out, string indent) {
//    out << indent << "sm" << re_id_map[this]
//	<< "->check()";
//}

// The version for parser2.cpp
void RE::emit(ostream& out, string indent) {
	out << "ret_state_" << this;
}

SingleRE::SingleRE(list<Expr*>* expr) : pred(expr) {}

//void SingleRE::emit(ostream& out, string indent) {
//    out << indent << "SingleRE";
//}

void SingleRE::emitInitCode(ostream& out, string indent) {
	out << indent << "new SingleRE(" << endl;
	emitPred(pred, out, indent+INDENT);
	out << endl << indent << ")";
}

void RE::emitDataStructure(ostream& out) {
	emitDataStructure(out, freeVariables.begin());
}

void RE::emitDataStructureType(ostream& out, int level) {
	if (freeVariables.empty()) {
		if (level>0)
		return;
		out << "int"
		<< " state_" << this << " = " 
		<<fsm->initState << ";" << endl;
		return;
	}

	string var = freeVariables.back();
	if (tree->name_to_id[var]+1 == level) {
		out << "int"
		<< " state_" << this << " = " 
		<<fsm->initState << ";" << endl;
	}
}

void RE::emitCheck(ostream& out, int level) {
	if (level < 0) {
		out << "bool ret_state_" << this 
		<< " = false;" << endl; 
		return;
	}

	if (freeVariables.empty()) {
		if (level==0) {
			out << "if (state" << ".state_" << this
			<< "=="
			<< *fsm->finalStates.begin()
			<< ") {" << endl;
			out << "ret_state_" << this << " = " 
			<< "true;" << endl << "}" << endl;
		}
	}

	string var = freeVariables.back();
	if (tree->name_to_id[var]+1 == level) {
		out << "if (state_" << level << ".state_" << this
		<< "=="
		<< *fsm->finalStates.begin()
		<< ") {" << endl;
		out << "ret_state_" << this << " = " 
		<< "true;" << endl << "}" << endl;
	}
}


void RE::emitDataStructureType(ostream& out, list<string>::iterator startit) {
	for (auto s = startit; 
	s != freeVariables.end();
	s++) {
		out << "map<int, ";
	}
	out << "int";
	for (auto s = startit; 
	s != freeVariables.end();
	s++) {
		out << ">";
	}
}

void RE::emitDataStructure(ostream& out, list<string>::iterator startit) {
	// normal state map
	emitDataStructureType(out, startit);
	out << " state_" << this << endl;

	// default state map
	emitDataStructureType(out, ++startit);
	out << " state_" << this << "_default" << endl;
}

/* 
* This version output a single function for returning state
* 
* */
//void RE::emitCheck(ostream& out) {
//    out << "bool check_state_" << this << "(Packet* last) {" << endl;
//    string nodeName = "state";
//
//    for (auto var : freeVariables) {
//	if (is_number(var))
//	    continue;
//	ostringstream convert;
//	convert << "state_" << tree->name_to_id[var]+1;
//	string subNodeName = convert.str();
//
//	out << "Node_" << tree->name_to_id[var]+1 
//	    << "& state_" << tree->name_to_id[var]+1
//	    << ";" << endl;
//
//	out << "if (" << var << " not in " 
//	    << nodeName << ".state_map) {" 
//	    << endl;
//	out << "state_" << tree->name_to_id[var]+1
//	    << " = " << nodeName << ".default_state;" 
//	    << endl;
//	out << "} else {" << endl
//	    << "state_" << tree->name_to_id[var]+1
//	    << " = " << nodeName 
//	    << ".state_map[" << var << "];"
//	    << endl;
//	out << "}" << endl;
//	nodeName = subNodeName;
//    }
//
//    out << "if (" << nodeName << ".state_" << this 
//	<< "==" << *fsm->finalStates.begin()
//	<< ") {" << endl
//	<< "return true;" << endl << "}" << endl;
//    out << "return false;" << endl;
//    out << "}" << endl << endl;
//}

void RE::emitUpdate(ostream& out) {
	emitUpdate(out, "state", tree->root); 
}


/*
* Only handles re's with equalities 
TODO:: general cases
*/
void RE::emitUpdate(ostream& out, string DSname, TreeNode *node) {
	cout << "RE update at Node: " << node->name << endl;

	// case 1: no free variables 
	if (freeVariables.empty()) {
		for (auto trans : node->transitions) {
			void* from = trans.first;
			void* to = trans.second;
			out << "if (" 
			<< DSname << ".state_" << this
			<< " == " << from << ") {" << endl;
			out << DSname << ".state_" << this
			<< " = " << to << ";" << endl;
			if (online)
			parent->emitUpdateChange(out, this, "old", "new");
			out << "}" << endl;
		}
		return;
	}

	string lastVar = freeVariables.back();


	if (node->tree->name_to_id[node->name]
			== node->tree->name_to_id[lastVar]+1) {
		for (auto trans : node->transitions) {
			void* from = trans.first;
			void* to = trans.second;
			out << "if (" 
			<< DSname << ".state_" << this
			<< " == " << from << ") {" << endl;
			out << DSname << ".state_" << this
			<< " = " << to << ";" << endl;
			if (online)
			parent->emitUpdateChange(out, this, "old", "new");
			out << "}" << endl;
		}
		return;
	}

	// If this node is not a variable for this RE,
	// then go through all its children
	if (!isIn(node->name, freeVariables)) {
		if (is_number(node->name)) {
			for (auto it=node->childrenMap.begin();
			it!=node->childrenMap.end();
			it++) {
				string field = it->first;
				TreeNode* child = it->second;
				emitUpdate(out, "state", child);
			}
			return;
		} else {
			//	    for (auto it=node->childrenMap.begin();
			//		    it!=node->childrenMap.end();
			//		    it++) {
			//		string field = it->first;
			//		TreeNode* child = it->second;
			//		emitUpdate(out, DSname, child);
			//	    }

			for (auto it=node->childrenMap.begin();
			it!=node->childrenMap.end();
			it++) {
				string field = it->first;
				TreeNode* child = it->second;

				// This is a hack, it assumes that this case 
				// appears only when using piping.
				// need to re-do when have time.
				ostringstream convert;
				convert << "state_";
				convert << 1+tree->name_to_id[node->name];
				emitUpdate(out, convert.str(), child);
			}

			return;
		}
	}

	if (is_number(node->name)) {
		for (auto it=node->childrenMap.begin();
		it!=node->childrenMap.end();
		it++) {
			string field = it->first;
			TreeNode* child = it->second;
			out << "if (" << field << " == " 
			<< node->name << ") { " << endl;
			emitUpdate(out, "state", child);
			out << "}" << endl;
		}
		return;
	}
	
	for (auto it=node->childrenMap.begin();
	it!=node->childrenMap.end();
	it++) {
		string field = it->first;
		// fork a branch
		out << "auto it" << tree->name_to_id[node->name] 
		<< " = "
		<< DSname << ".state_map.find(" << field << ");" << endl;
		out << "if (it" << tree->name_to_id[node->name]
		<< " == " << DSname << ".state_map.end()) { " << endl;
		out << "it" << tree->name_to_id[node->name]
		<< " = "
		<< DSname << ".state_map.insert({" 
		<< field << ", " << DSname << ".default_state}).first" 
		<< ";" << endl;
		out << "}" << endl; 
		//    }
		//
		//    for (auto it=node->childrenMap.begin();
		//	    it!=node->childrenMap.end();
		//	    it++) {
		//	string field = it->first;
		TreeNode* child = it->second;

		ostringstream nameConvert;
		nameConvert << "state_" << 1+node->tree->name_to_id[node->name];
		string subMapName = nameConvert.str();

		ostringstream typeConvert;
		// TODO
		//	emitDataStructureType(typeConvert, ++freeVariables.begin());
		//	string subMapType = typeConvert.str();
		typeConvert << "Node_" << tree->name_to_id[node->name]+1;
		string subMapType = typeConvert.str();
		out << subMapType << "& " << subMapName << " = " 
		<< "it" << tree->name_to_id[node->name]
		<< ".second;" << endl;
		emitUpdate(out, subMapName, child);
	}


	//    for (auto it=node->childrenMap.begin();
	//	    it!=node->childrenMap.end();
	//	    it++) {
	//	string field = it->first;
	//	TreeNode* child = it->second;
	//
	//	out << "if (" << field << " in " << DSname << ") { " <<endl;
	//
	//	// Type and name for submap 
	//	ostringstream typeConvert;
	//	for (auto it = node->tree->name_to_id.find(node->name); 
	//		it != --node->tree->name_to_id.end();
	//		it++) {
	//	    typeConvert << "map<int, ";
	//	}
	//	typeConvert << "int";
	//	for (auto it = node->tree->name_to_id.find(node->name); 
	//		it != --node->tree->name_to_id.end();
	//		it++) {
	//	    typeConvert << ">";
	//	}
	//	string subMapType = typeConvert.str();
	//
	//	ostringstream nameConvert;
	//	nameConvert << "state_" << node;
	//	string subMapName = nameConvert.str();
	//	out << subMapType << "& " << subMapName << " = " << DSname << "[" << field << "];" << endl;
	//	emitUpdate(out, subMapName, child);
	//	out << "} else {" << endl;
	//	// else
	//	out << subMapType << " " << subMapName << " = ";
	//	out << "default" ;
	//        out << ";" << endl;
	//	emitUpdate(out, subMapName, child);
	//	out << DSname << "["<<field<<"] = "<< subMapName << ";" <<endl;
	//	out << "}" << endl;
	//    }
}

void RE::emitResetState(ostream& out) {
	if (freeVariables.empty()) {
		out << "state" << ".state_" << this
		<< " = " << fsm->initState << ";" << endl;
		return;
	}
	string lastVar = freeVariables.back();
	int level = tree->name_to_id[lastVar]+1;
	out << "state_" << level << ".state_" << this
	<< " = " << fsm->initState << ";" << endl;
}

FSM* SingleRE::toFSM(Tree *tree) {
	list<Expr*> emptyPred;
	if (pred == NULL) {
		pred = new list<Expr*>();
	}

	set<TreeNode*> predNodes = addPredicateAt(tree->root, *pred);

	fsm = new FSM();
	State *state1 = fsm->addInitState();
	State *state2 = fsm->addFinalState();
	fsm->addTransition(state1, predNodes, state2);

	cout << endl << "single" << endl;
	fsm->print();


	return fsm;
}

void emitPred(list<Expr*>* pred, ostream& out, string indent) {
	if (pred==NULL) {
		out << indent << "new Pred(" ;
		out << "true";
		out << ")";
	} else {
		list<Expr*>::iterator it = pred->begin();
		out << indent << "new SymbolicPred{" << endl;
		out << indent+INDENT << "{";
		(*it)->emitRevPred(out);
		out << "}";
		it++;
		for (; it!=pred->end(); it++) {
			out << "," << endl;
			out << indent+INDENT << "{";
			(*it)->emitRevPred(out);
			out << "}";
		}
		out << endl;
		out << indent << "}";
	}
}

ConcatRE::ConcatRE(RE* re1, RE* re2) : re1(re1), re2(re2) {}

//void ConcatRE::emit(ostream& out, string indent) {
//    out << indent << "sm" << re_id_map[this]
//	<< "->check()";
//}

void ConcatRE::emitInitCode(ostream& out, string indent) {
	out << indent << "new ConcatRE(" << endl;
	re1->emitInitCode(out, indent+INDENT);
	out << "," << endl;
	re2->emitInitCode(out, indent+INDENT);
	out << endl << indent <<  ")";
}

FSM* ConcatRE::toFSM(Tree *tree) {
	cout << endl << "concat" << endl;
	FSM *fsm1 = re1->toFSM(tree);
	FSM *fsm2 = re2->toFSM(tree);

	fsm1->print();
	fsm2->print();

	for (auto state : fsm2->states) {
		fsm1->states.insert(state);
	}

	for (auto state : fsm1->finalStates) {
		fsm1->addEpsilonTransition(state, fsm2->initState);
	}

	fsm1->finalStates.clear();
	for (auto state : fsm2->finalStates) {
		fsm1->finalStates.insert(state);
	}

	cout << "before determinized" << endl;
	fsm1->print();
	fsm = determinize(fsm1);
	cout << "after determinized" << endl;
	fsm->print();
	fsm->minimize();
	cout << "after minimized" << endl;
	fsm->print();
	return fsm;
}

StarRE::StarRE(RE* re) : re(re) {}

//void StarRE::emit(ostream& out, string indent) {
//    re->emitInitCode(out);
//    out << indent << "StarRE";
//}

void StarRE::emitInitCode(ostream& out, string indent) {
	out << indent << "new StarRE(" << endl;
	re->emitInitCode(out, indent+INDENT);
	out << endl << indent <<  ")";
}

FSM* StarRE::toFSM(Tree *tree) {
	FSM *fsm1 = re->toFSM(tree);
	for (auto state : fsm1->finalStates) {
		fsm1->addEpsilonTransition(state, fsm1->initState);
	}
	State *state = fsm1->initState;
	State *newState = fsm1->addInitState(true);

	cout << endl << "star" << endl;
	fsm1->addEpsilonTransition(newState, state);
	cout << "Before deter" << endl;
	fsm1->print();

	fsm = determinize(fsm1);
	cout << "After deter" << endl;
	fsm->print();

	fsm->minimize();
	cout << "After minimize" << endl;

	fsm->print();

	return fsm;
}

UnionRE::UnionRE(RE* re1, RE* re2) : re1(re1), re2(re2) {}

//void UnionRE::emit(ostream& out, string indent) {
//    out << indent << "UnionRE";
//}

void UnionRE::emitInitCode(ostream& out, string indent) {
	out << "new UnionRE(";
	re1->emitInitCode(out);
	out << ",";
	re2->emitInitCode(out);
	out << ")";
}

void SingleRE::getFreeVariables() {
	if (pred == NULL)
	return;
	for (auto e : *pred) {
		ostringstream convert;

		BiopExpr* biExpr = (BiopExpr*)(e);
		string s = typeid(biExpr->right).name();
		cout<<"TYPE - "<<s<<endl; 
		
		if( PlusExpr* right_plus = dynamic_cast< PlusExpr* >( biExpr->right ) )
		{
			cout<<"Plus Expression in Single RE\n";
			MinusExpr* left_minus = new MinusExpr((Expr*)biExpr->left, (Expr*)right_plus->right);
			biExpr->left = left_minus;
			right_plus->left->emit(convert);
			string name = convert.str();
			biExpr->right = new IdExpr(name);
		}
		else if( MinusExpr* right_minus = dynamic_cast< MinusExpr* >( biExpr->right ) )
		{
			cout<<"Minus Expression in Single RE\n";
			PlusExpr* left_plus = new PlusExpr((Expr*)biExpr->left, (Expr*)right_minus->right);
			biExpr->left = left_plus;
			right_minus->left->emit(convert);
			string name = convert.str();
			biExpr->right = new IdExpr(name);
		}
			convert.str("");
			convert.clear();
			IdExpr* right = (IdExpr*)biExpr->right; 
			right->emit(convert);
			string name = convert.str();
			bool isNew = true;
			for (string s : freeVariables) {
				if (s.compare(name) == 0) {
					isNew = false;
					break;
				}
			}

			if (isNew) {
				cout << "new variable " << name << endl;
				freeVariables.push_back(name);
			}
		
	} 
}

void ConcatRE::getFreeVariables() {
	re1->getFreeVariables();
	re2->getFreeVariables();

	freeVariables.insert(freeVariables.end(),
	re1->freeVariables.begin(),
	re1->freeVariables.end()
	);

	for (auto s : re2->freeVariables) {
		bool isIn = false;
		for (auto s2 : freeVariables)
		if (s2.compare(s) == 0) {
			isIn = true;
			break;
		}
		if (!isIn) {
			freeVariables.push_back(s);
		}
	}
}

void StarRE::getFreeVariables() {
	re->getFreeVariables();
	freeVariables.insert(freeVariables.end(),
	re->freeVariables.begin(),
	re->freeVariables.end()
	);
}