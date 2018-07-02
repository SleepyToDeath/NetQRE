#ifndef NFA_HPP
#define NFA_HPP

#include <vector>
#include <set>
#include <map>
#include <iostream>

const char Epsilon = "#";

typedef NFAIt std::set<NFAState*>::iterator;

class NFA {
	public:
	NFA() {
	}

	NFA(NFA* src) {
		std::map<NFAState*, NFAState*> m;
		for (NFAIt i = src->states.begin(); i!=src->states.end(); i++)
		{
			NFAState* new_state = new NFAState*();
			new_state->transition = (*i)->transition;
			states.insert(new_state);
			m[(*i)] = new_state;
		}
		for (NFAIt i = states.begin(); i!=states.end(); i++)
		{
			NFAState* new_state = (*i);
			for (std::map<char, std::set<NFAState*> >::iterator j = new_state->transitions.begin(); j<new_state->transitions.end(); j++)
			{
				char ch = j->first;
				std::set<NFAState*> new_transition = j->second;
				for (NFAIt k = new_transition.begin(); k!=new_transition.end(); k++)
					(*k) = m[(*k)];
				new_state->transitions[ch] = new_transition;
			}
		}
		for (NFAIt i = src->start_states.begin(); i!=src->start_states.end(); i++)
			start_states.insert(m[(*i)]);
		for (NFAIt i = src->accept_states.begin(); i!=src->accept_states.end(); i++)
			accept_states.insert(m[(*i)]);
	}

	std::set<NFAState*> find_neighbours(std::set<NFAState*> the_set) {
		std::set<NFAState*> neighbours;
		for (NFAIt i = the_set.begin(); i!=the_set.end(); i++)
			neighbours.insert((*i)->transitions[Epsilon].begin(), (*i)->transitions[Epsilon].end());
		neighbours.insert(the_set.begin(), the_set.end());
		if (neighbours.size() == the_set.size())
			return neighbours;
		else
			return find_neighbours(neighbours);
	}

	std::set<NFAState*> transition(std::set<NFAState*> current, char s) {
		std::set<NFAState*> next;
		for (NFAIt i = current.begin(); i!=current.end(); i++)
			next.insert((*i)->transition[s].begin, (*i)->transition[s].end());
		return next;
	}

	bool accept(std::string input) {
		active_states = find_neighbours(start_states);

		for (int i=0; i<input.size(); i++)
			active_states = find_neighbours(transition(active_states, input[i]));

		for (NFAIt i = active_states.begin(); i!=active_states.end(); i++)
			if (accept_states.find(*i) != accept_states.end())
				return true;

		return false;
	}

	std::set<NFAState*> active_states;
	std::set<NFAState*> start_states;
	std::set<NFAState*> accept_states;
	std::set<NFAState*> states;
};

class NFAState {
	public:
	std::map<char, std::set<NFAState*> > transitions;
};

#endif
