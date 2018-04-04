#include "fsm.h"
//#include "packet.h"

State::State(bool isFinal) : isFinal(isFinal) {}

State* FSM::addInitState(bool isFinal) {
    initState = addState(isFinal);
    return initState;
}

State* FSM::addFinalState() {
    return addState(true);
}

State* FSM::addState(bool isFinal) {
    State* state = new State(isFinal);
    states.insert(state);
    if (isFinal)
	finalStates.insert(state);
    return state;
}

ostream& operator<<(ostream& os, const State& state) {
    os << state.id;
    return os;
}

ostream& operator<<(ostream& os, const State* &state) {
    os << state->id;
    return os;
}

void FSM::addTransition(State* from, 
	set<TreeNode*>& predNodes,
	State* to) {
    for (auto node : predNodes) {
	to->inTransitions.push_back(
	    new pair<State*, TreeNode*>(
		from, node
	    )
	);
	from->outTransitions.push_back(
	    new pair<State*, TreeNode*>(
		to, node
	    )
	);
    }
}

void FSM::addEpsilonTransition(State* from, State* to) {
    to->inTransitions.push_back(
	new pair<State*, TreeNode*>(
	    from, NULL
	)
    );
    from->outTransitions.push_back(
	new pair<State*, TreeNode*>(
	    to, NULL
	)
    );
}

set<State*>& FSM::getFinalStates() {
    return finalStates;
}

int FSM::size() {
    return states.size();
}

void FSM::print() {
    cout << "------------- FSM ---------------" << endl;
    for (auto state : states) {
	for (auto trans : state->outTransitions) {
	    cout << *state 
		<< " "
		<< trans->second
		<< " -> "
		<< *(trans->first)
		<< endl;
	}
    }
    // print initial states
    cout << "Initial: ";
    cout << *initState;
    cout << endl;

    // print final states
    cout << "Final: ";
    for (auto state : finalStates) {
	    cout << *state 
		<< " ";
    }
    cout << endl;
    cout << "------------- FSM ---------------" << endl;
}

//void FSM::updateState(Packet&);
//bool FSM::check() {
//}

set<State*>* State::getClosedStates() {
    list<State*> *closedStates = new list<State*>();
    closedStates->push_back(this);

    auto it = closedStates->begin();

    for ( ; it !=  closedStates->end(); it++) {
	State *state = *it;

	for (auto outTran : state->outTransitions) {
	    // if this is an epsilon transition
	    if (outTran->second == NULL) {
		State *newstate = outTran->first;

		// if this is a new state
		bool isNew = true;
		for (auto oldstate : *closedStates)
		    if (oldstate == newstate) {
			isNew = false;
			break;
		    }
		if (isNew)
		    closedStates->push_back(newstate);
	    }
	}
    }

    set<State*> *ret = new set<State*>(closedStates->begin(), closedStates->end());
    return ret;
}

FSM* determinize(FSM* oldFSM) {
    FSM *fsm = new FSM();
    //fsm->predManager = oldFSM->predManager;
    map<State*, set<State*>*> stateMap;

    State *newInitState = fsm->addInitState();
    stateMap[newInitState] = oldFSM->initState->getClosedStates();

int pause;
cout << "new init state" << endl;
cout << newInitState << ": ";
for (auto state : *stateMap[newInitState]) {
    cout << state << " ";
}
cout << endl;

    list<State*> newStates;
    newStates.push_back(newInitState);

    auto it = newStates.begin();
    for ( ; it != newStates.end(); it++) {
	State *newFromState = *it;
	set<State*> *oldFromStates = stateMap[newFromState];

	// collect all transition predicates
	set<TreeNode*> allTransitionPreds;
	for (auto oldstate : *oldFromStates) {
	    for (auto trans : oldstate->outTransitions) {
		if (trans->second != NULL)
		    allTransitionPreds.insert(trans->second);
	    }
	}

	for (auto pred : allTransitionPreds) {
	    cout << "Pred " << pred << endl;
	    set<State*> *oldToStates = new set<State*>();
	    for (auto oldstate : *oldFromStates) {
		// if exists oldstate * pred -> nextstate
		for (auto trans : oldstate->outTransitions) {
		    if (trans->second == pred) {
			State* nextstate = trans->first;
			set<State*> *closedStates = nextstate->getClosedStates();
			oldToStates->insert(
			    closedStates->begin(),
			    closedStates->end()
			);
		    }
		}
	    }


	    State *newToState = NULL;
	    for (auto state : newStates) {
		// if this is an exsiting new state

cout << "oldToStates" << endl;
for (auto sstate : *oldToStates) {
    cout << sstate << " ";
}
cout << endl;

cout << endl<< "TEST" << endl;
for (auto sstate : *stateMap[state]) {
    cout << sstate << " ";
}
cout << endl;

		if (*stateMap[state] == *oldToStates) {
		    cout << "DFSDFD" << endl;
		    newToState = state;
		    break;
		}
	    }

	    if (newToState == NULL) {
		newToState = fsm->addState(false);
		stateMap[newToState] = oldToStates;
		newStates.push_back(newToState);
	    }

cout << newToState << ": ";
for (auto state : *stateMap[newToState]) {
    cout << state << " ";
}


	    newFromState->outTransitions.push_back(
		new pair<State*, TreeNode*>(newToState, pred)
	    );

	    newToState->inTransitions.push_back(
		new pair<State*, TreeNode*>(newFromState, pred)
	    );
	}
    }
    
    // set final states
    for (auto newState : newStates) {
	for (auto oldFinalState : oldFSM->finalStates) {
	    if (stateMap[newState]->find(oldFinalState) != stateMap[newState]->end()) {
		newState->isFinal = true;
		fsm->finalStates.insert(newState);
		break;
	    }
	}
    }

    return fsm;
}

void FSM::minimize() {
    map<pair<State*, State*>, int> distinct;
    list<State*> stateList;

    // put all states into a list
    for (auto p : states) 
	stateList.push_back(p);

    // init distinct table
    for (auto it1 = stateList.begin(); it1 != stateList.end(); it1++) {
	auto it2 = it1;
	it2++;
	for ( ; it2 != stateList.end(); it2++) {
	    State* p = *it1;
	    State* q = *it2;
	    if (p->isFinal && !q->isFinal) {
		distinct[{p,q}] = 1;
	    } else if (!p->isFinal && q->isFinal) {
		distinct[{p,q}] = 1;
	    } else 
		distinct[{p,q}] = 0;
	}
    }

    bool changed = true;
    while (changed) {
	changed = false;
	for (auto it1 = stateList.begin(); it1 != stateList.end(); it1++) {
	    State* p = *it1;
	    auto it2 = it1;
	    it2++;

	    set<TreeNode*> preds1;
	    for (auto trans : p->outTransitions) {
		preds1.insert(trans->second);
	    }

	    for ( ; it2 != stateList.end(); it2++) {
		State* q = *it2;
		if (distinct[{p,q}] == 1)
		    continue;

		set<TreeNode*> preds2;
		for (auto trans : q->outTransitions) {
		    preds2.insert(trans->second);
		}

		if (preds1!=preds2) {
		    distinct[{p,q}] = 1;
		    changed = true;
		    continue;
		}
		for (auto trans1 : p->outTransitions) {
		    State* to = trans1->first;
		    TreeNode* pred = trans1->second;
		    for (auto trans2 : q->outTransitions) {
			if (trans2->second != pred)
			    continue;
			if (trans2->first != to) {
			    distinct[{p,q}] = 1;
			    changed = true;
			    break;
			}
		    }
		}
	    }
	}
    }

    // merge identical states
    for (auto it1 = stateList.begin(); it1 != stateList.end(); it1++) {
	State* p = *it1;

	auto it2 = stateList.end();
	it2--;
	for (; it2 != it1; it2--) {
	    State* q = *it2;
	    if (distinct[{p,q}]==1)
		continue;

	    for (auto trans : q->inTransitions) {
		State* from = trans->first;
		TreeNode* pred = trans->second;

		if (from == q)
		    continue;

		for (auto transp : from->outTransitions) {
		    if (transp->second == pred) {
			transp->first = p;
			break;
		    }
		}

		p->inTransitions.push_back(
		    new pair<State*, TreeNode*>(from, pred)
		);
	    }

	    if (q == initState)
		initState = p;
	    stateList.erase(it2);
	    this->states.erase(q);
	    this->finalStates.erase(q);
	}
    }
}

void FSM::populatePredToTree() {
    for (auto state : states) {
	for (auto trans : state->outTransitions) {
	    trans->second->transitions.push_back(
		    pair<void*,void*>(
			(void*)state,
			(void*)trans->first
			)
		    );
	}
    }
}

void FSM::assignStateId() {
    int id = 0;
    for (auto state : states)
	state->id = id++;
}
