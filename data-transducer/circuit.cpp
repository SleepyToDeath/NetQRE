#include "circuit.h"

namespace DT {

	Circuit::Circuit()
	{

	}

	int Circuit::add_gate(Gate* g, GateType t)
	{
		int id = 0;
		switch (t)
		{
			case GATE:
				gate.push_back(g);
				id = gate.size()-1;
				break;

			case STREAM_IN:
				streami.push_back(g);
				id = gate.size()-1;
				break;

			case STREAM_OUT:
				streamo.push_back(g);
				id = gate.size()-1;
				break;

			case STATE_IN:
				statei.push_back(g);
				id = gate.size()-1;
				break;

			case STATE_OUT:
				stateo.push_back(g);
				id = gate.size()-1;
				break;

				default
					break;
		}
		return id * (int)NUM_GATE_TYPE + (int)t;
	}

	Gate* Circuit::get_gate(int id)
	{
		GateType t = (GateType)(id % (int)NUM_GATE_TYPE);
		int index = id / (int)NUM_GATE_TYPE;
		switch(t)
		{
			case GATE:
				return gate[index];
				break;

			case STREAM_IN:
				return streami[index];
				break;

			case STREAM_OUT:
				return streamo[index];
				break;

			case STATE_IN:
				return statei[index];
				break;

			case STATE_OUT:
				return stateo[index];
				break;

				default
					return nullptr;
				break;
		}
	}

	vector<int> Circuit::get_state_out()
	{
		vector<int> out;
		for (int i=0; i<stateo.size(); i++)
		{
			out.push_back(stateo[i]->output(CMB));
		}
		return out;
	}

	void Circuit::set_state_in(vector<int> states)
	{
		for (int i=0; i<states.size(); i++)
		{
			statei[i]->set_value(states[i]);
		}
	}

	vector<int> Circuit::get_stream_out()
	{
		vector<int> out;
		for (int i=0; i<streamo.size(); i++)
		{
			out.push_back(streamo[i]->output(CMB));
		}
		return out;
	}

	void Circuit::set_stream_in(int val)
	{
		for (int i=0; i<streami.size(); i++)
		{
			streami[i]->set_value(val);
		}
	}

	void Circuit::reset()
	{
		for (int i=0; i<streami.size(); i++)
			streami[i]->reset();
		for (int i=0; i<statei.size(); i++)
			statei[i]->reset();
		for (int i=0; i<gate.size(); i++)
			gate[i]->reset();
		for (int i=0; i<stateo.size(); i++)
			stateo[i]->reset();
	}

	void Circuit::tick()
	{
		for (int i=0; i<streami.size(); i++)
			streami[i]->posedge();
		for (int i=0; i<statei.size(); i++)
			statei[i]->posedge();
		for (int i=0; i<gate.size(); i++)
			gate[i]->posedge();
		for (int i=0; i<stateo.size(); i++)
			stateo[i]->posedge();
	}

	int Circuit::combine_char_union(Circuit* c)
	{
		return combine_char_parallel(c);
	}

	int Circuit::combine_char_parallel(Circuit* c)
	{
		for (int i=0; i<c->statei.size(); i++)
			statei.push_back(c->statei[i]);
		for (int i=0; i<c->stateo.size(); i++)
			stateo.push_back(c->stateo[i]);
		for (int i=0; i<c->streami.size(); i++)
			streami.push_back(c->streami[i]);
		for (int i=0; i<c->gate.size(); i++)
			gate.push_back(c->gate[i]);
		return 0;
	}

	int Circuit::combine_char_concatenation(Circuit* c)
	{
		return combine_char_parallel(c);
	}

	int Circuit::combine_char_star()
	{
		return 0;
	}

	int Circuit::combine_epsilon_union(Circuit* c)
	{
		combine_char_parallel(c);
		for (int i=0; i<
	}

	int Circuit::combine_epsilon_parallel(Circuit* c)
	{

	}

	int Circuit::combine_epsilon_concatenation(Circuit* c)
	{

	}

	int Circuit::combine_epsilon_star()
	{

	}


}
