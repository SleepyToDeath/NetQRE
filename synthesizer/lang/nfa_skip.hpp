#ifndef NFA_SKIP_HPP
#define NFA_SKIP_HPP

#include "nfa.hpp"

class NFASkip: public NFA {
	public:

	NFASkip():NFA() {
	}

	NFASkip(shared_ptr<NFASkip> src):NFA(src) {
	}

	/* can skip is actually a syntactic sugar */
	bool accept(std::string input, std::vector<bool> can_skip) {
		active_states = find_neighbours(start_states);

		for (int i=0; i<input.size(); i++)
		{
			std::set<shared_ptr<NFAState> > backup_active_states = active_states;
			active_states = find_neighbours(transition(active_states, input[i]));
			if (can_skip[i])
			{
				/* can skip this character */
				active_states.insert(backup_active_states.begin(), backup_active_states.end());
			}
		}

		for (NFAIt i = active_states.begin(); i!=active_states.end(); i++)
			if (accept_states.find(*i) != accept_states.end())
				return true;

		return false;
	}

};

#endif
