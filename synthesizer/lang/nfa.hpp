#ifndef NFA_HPP
#define NFA_HPP

#include <vector>
#include <set>
#include <map>
#include <iostream>

const char Epsilon = '#';

class NFAState {
	public:
	std::map<char, std::set<shared_ptr<NFAState> > > transitions;
};

typedef std::set< shared_ptr<NFAState> >::iterator NFAIt;

class NFA {
	public:
	NFA() {
	}


	NFA(shared_ptr<NFA> src) {
		std::map<shared_ptr<NFAState>, shared_ptr<NFAState> > m;
		for (NFAIt i = src->states.begin(); i!=src->states.end(); i++)
		{
			auto new_state = shared_ptr<NFAState>(new NFAState());
			new_state->transitions = (*i)->transitions;
			states.insert(new_state);
			m[(*i)] = new_state;
		}
		for (NFAIt i = states.begin(); i!=states.end(); i++)
		{
			auto new_state = (*i);
			std::map<char, std::set<shared_ptr<NFAState> > > new_transitions;
			for (std::map<char, std::set<shared_ptr<NFAState> > >::iterator j = new_state->transitions.begin(); j!=new_state->transitions.end(); j++)
			{
				char ch = j->first;
				std::set<shared_ptr<NFAState> > new_transition;
				for (NFAIt k = j->second.begin(); k!=j->second.end(); k++)
					new_transition.insert(m[(*k)]);
				new_transitions[ch] = new_transition;
			}
			new_state->transitions = new_transitions;
		}
		for (NFAIt i = src->start_states.begin(); i!=src->start_states.end(); i++)
			start_states.insert(m[(*i)]);
		for (NFAIt i = src->accept_states.begin(); i!=src->accept_states.end(); i++)
			accept_states.insert(m[(*i)]);
	}

	std::set<shared_ptr<NFAState> > find_neighbours(std::set<shared_ptr<NFAState> > the_set) {
		std::set<shared_ptr<NFAState> > neighbours;
		for (NFAIt i = the_set.begin(); i!=the_set.end(); i++)
			neighbours.insert((*i)->transitions[Epsilon].begin(), (*i)->transitions[Epsilon].end());
		neighbours.insert(the_set.begin(), the_set.end());
		if (neighbours.size() == the_set.size())
			return neighbours;
		else
			return find_neighbours(neighbours);
	}

	std::set<shared_ptr<NFAState> > transition(std::set<shared_ptr<NFAState> > current, char s) {
		std::set<shared_ptr<NFAState> > next;
		for (NFAIt i = current.begin(); i!=current.end(); i++)
			next.insert((*i)->transitions[s].begin(), (*i)->transitions[s].end());
		return next;
	}

	bool accept(std::string input) {
		active_states = find_neighbours(start_states);

		for (int i=0; i<input.size(); i++)
		{
			active_states = find_neighbours(transition(active_states, input[i]));
		}

		for (NFAIt i = active_states.begin(); i!=active_states.end(); i++)
			if (accept_states.find(*i) != accept_states.end())
				return true;

		return false;
	}

	std::set<shared_ptr<NFAState> > active_states;
	std::set<shared_ptr<NFAState> > start_states;
	std::set<shared_ptr<NFAState> > accept_states;
	std::set<shared_ptr<NFAState> > states;
};

#endif
