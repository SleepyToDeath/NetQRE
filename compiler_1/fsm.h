#ifndef _FSM_H
#define _FSM_H

#include <vector>
#include <set>
#include <list>
#include <tuple>
#include <iostream>
//#include "packet.h"
#include "predtree.h"
//#include "predicate.h"

using namespace std;

class State {
    public:
	int id;
	bool isFinal;
	list<pair<State*, TreeNode*>*> inTransitions;
	list<pair<State*, TreeNode*>*> outTransitions;

    public:
	State(bool=false);
	set<State*>* getClosedStates();
};

ostream& operator<<(ostream& os, const State&);

class FSM {
    public:
	set<State*> states;
	set<State*> finalStates;
	State* initState;
	int nState;

    public:
	//FSM(int);
	State* addInitState(bool=false);
	State* addFinalState();
	// add a new state, with whether it is final
	State* addState(bool=false);
	//void addTransition(State*, SymbolicPred*, State*);
	void addTransition(State*, 
	    set<TreeNode*>&,
	    State*);
	void addEpsilonTransition(State*, State*);
	void minimize();

	int size();
	void assignStateId();
	//void deleteState(state);
//	int getNumStates();
	set<State*>& getFinalStates();
//	void setFinalState(int);
//	void clearFinalState();
//	void updateState(Packet&);
//	bool checkState();
	void print();
	void populatePredToTree();
};

FSM* determinize(FSM*);
#endif
