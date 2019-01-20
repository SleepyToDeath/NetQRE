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

		return accept_fast(input, can_skip);

		active_states = find_neighbours(start_states);

		for (int i=0; i<input.size(); i++)
		{
			unordered_set<shared_ptr<NFAState> > backup_active_states = active_states;
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

	bool accept_fast(std::string input, std::vector<bool> can_skip) {

		example_count ++;

		auto visited = shared_ptr<BitSet>(new BitSet(states.size()));

		prepare_runtime();

		runtime_active_states = find_neighbours_fast(runtime_start_states);

		for (int i=0; i<input.size(); i++)
		{
			auto backup_active_states = shared_ptr<BitSet>(new BitSet(runtime_active_states));
			runtime_active_states = find_neighbours_fast( transition_fast(runtime_active_states, input[i]) );
			if (can_skip[i])
			{
				runtime_active_states->merge(backup_active_states);
			}

			visited->merge(runtime_active_states);
		}

		double this_round_rate= ((double)(visited->count())) / ((double)(states.size()));

		utility_rate = (utility_rate * (double)(example_count-1) + this_round_rate) / (double)example_count;

		return runtime_active_states->intersect(runtime_accept_states);

	}

	double utility_rate = 1.0;
	int example_count = 0;

};

#endif
