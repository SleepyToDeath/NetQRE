#ifndef NFA_HPP
#define NFA_HPP

#include <vector>
#include <unordered_set>
#include <map>
#include <iostream>

using std::unordered_set;
using std::vector;

const char Epsilon = '#';

class BitSet {
	public:

	BitSet(unsigned long size) {
		bits = vector<unsigned long>(size/int_size + 1, 0);
		this->size = size;
	}

	BitSet(shared_ptr<BitSet> src) {
		bits = src->bits;
		size = src->size;
	}


	void insert(unsigned long index) {
		unsigned long a = index / int_size;
		unsigned long b = index % int_size;
		bits[a] = bits[a] | (1<<b);
	}

	bool get(unsigned long index) {
		unsigned long a = index / int_size;
		unsigned long b = index % int_size;
		return (bits[a] & (1<<b));
	}

	/* return if the intersection with src is empty ([!] but won't apply the intersection) */
	/* [!] The sizes are assumed to be the same */
	bool intersect(shared_ptr<BitSet> src) {
		unsigned long isize = bits.size();
		bool flag = false;
		for (int i=0; i<isize; i++)
			flag = flag || (bits[i] & src->bits[i]);
		return flag;
	}

	/* merge bits from src to this */
	/* [!] The sizes are assumed to be the same */
	void merge(shared_ptr<BitSet> src) {
		unsigned long isize = bits.size();
		for (int i=0; i<isize; i++)
			bits[i] = bits[i] | src->bits[i];
	}

	bool equal(shared_ptr<BitSet> src) {
		for (int i=0; i<bits.size(); i++)
			if (bits[i]!=src->bits[i])
				return false;
		return true;
	}

	/* number of active states */
	unsigned long count() {
		unsigned long c = 0;
		for (int i=0; i<size; i++)
			if (get(i))
				c++;
		return c;
	}

	static const unsigned long int_size = sizeof(int)*8;

	unsigned long size;

	vector<unsigned long> bits;

};

class NFAState {
	public:
	unsigned long id;
	std::map<char, unordered_set<shared_ptr<NFAState> > > transitions;
};

class NFARuntimeState {
	public:
	unsigned long id;
	vector<shared_ptr<BitSet> > transitions;
	shared_ptr<NFAState> origin;
};

typedef unordered_set< shared_ptr<NFAState> >::iterator NFAIt;

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
			std::map<char, unordered_set<shared_ptr<NFAState> > > new_transitions;
			for (std::map<char, unordered_set<shared_ptr<NFAState> > >::iterator j = new_state->transitions.begin(); j!=new_state->transitions.end(); j++)
			{
				char ch = j->first;
				unordered_set<shared_ptr<NFAState> > new_transition;
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

	unordered_set<shared_ptr<NFAState> > find_neighbours(unordered_set<shared_ptr<NFAState> > the_set) {
		unordered_set<shared_ptr<NFAState> > neighbours;
		for (NFAIt i = the_set.begin(); i!=the_set.end(); i++)
			neighbours.insert((*i)->transitions[Epsilon].begin(), (*i)->transitions[Epsilon].end());
		neighbours.insert(the_set.begin(), the_set.end());
		if (neighbours.size() == the_set.size())
			return neighbours;
		else
			return find_neighbours(neighbours);
	}

	shared_ptr<BitSet> find_neighbours_fast(shared_ptr<BitSet> the_set) {
		auto neighbours = shared_ptr<BitSet>(new BitSet(the_set));
		for (int i=0; i<the_set->size; i++)
			if (the_set->get(i) && runtime_states[i]->transitions[Epsilon]!=nullptr)
				neighbours->merge(runtime_states[i]->transitions[Epsilon]);
		if (neighbours->equal(the_set))
			return neighbours;
		else
			return find_neighbours_fast(neighbours);
	}

	unordered_set<shared_ptr<NFAState> > transition(unordered_set<shared_ptr<NFAState> > current, char s) {
		unordered_set<shared_ptr<NFAState> > next;
		for (NFAIt i = current.begin(); i!=current.end(); i++)
			next.insert((*i)->transitions[s].begin(), (*i)->transitions[s].end());
		return next;
	}

	shared_ptr<BitSet> transition_fast( shared_ptr<BitSet> current, char s) {
		shared_ptr<BitSet> next = shared_ptr<BitSet>(new BitSet(current->size));
		for (int i=0; i<current->size; i++)
			if (current->get(i) && runtime_states[i]->transitions[s]!=nullptr)
				next->merge(runtime_states[i]->transitions[s]);
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

	unordered_set<shared_ptr<NFAState> > active_states;
	unordered_set<shared_ptr<NFAState> > start_states;
	unordered_set<shared_ptr<NFAState> > accept_states;
	unordered_set<shared_ptr<NFAState> > states;

	void prepare_runtime() {
		runtime_states.clear();
		int count = 0;
		for (NFAIt i = states.begin(); i!=states.end(); i++)
		{
			auto tmp = shared_ptr<NFARuntimeState>(new NFARuntimeState());
			(*i)->id = count;
			tmp->id = count;
			tmp->origin = (*i);
			tmp->transitions = vector<shared_ptr<BitSet> >(256, nullptr);
			runtime_states.push_back(tmp);
			count++;
		}

		for (int i=0; i<runtime_states.size(); i++)
		{
			auto origin = runtime_states[i]->origin;
			for (auto j = origin->transitions.begin(); j!=origin->transitions.end(); j++)
			{
				runtime_states[i]->transitions[j->first] = shared_ptr<BitSet>(new BitSet(states.size()));
				for (auto k = j->second.begin(); k!=j->second.end(); k++)
					runtime_states[i]->transitions[j->first]->insert((*k)->id);
			}
		}

		runtime_start_states = shared_ptr<BitSet>(new BitSet(states.size()));
		for (NFAIt i = start_states.begin(); i!= start_states.end(); i++)
		{
			runtime_start_states->insert((*i)->id);
		}

		runtime_accept_states = shared_ptr<BitSet>(new BitSet(states.size()));
		for (NFAIt i = accept_states.begin(); i!= accept_states.end(); i++)
		{
			runtime_accept_states->insert((*i)->id);
		}
	}

	vector<shared_ptr<NFARuntimeState> > runtime_states;
	shared_ptr<BitSet> runtime_start_states;
	shared_ptr<BitSet> runtime_accept_states;
	shared_ptr<BitSet> runtime_active_states;
};

#endif
