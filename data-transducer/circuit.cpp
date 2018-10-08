#include "circuit.h"

using std::shared_ptr;
using std::vector;

namespace DT {

	Circuit::Circuit()
	{

	}

	int Circuit::add_gate(shared_ptr<Gate> g, GateType t)
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
				id = streami.size()-1;
				break;

			case STATE_IN:
				statei.push_back(g);
				id = statei.size()-1;
				break;

			case STATE_IN_INIT:
				stateii.push_back(g);
				id = stateii.size()-1;
				break;

			case STATE_IN_FINAL:
				stateif.push_back(g);
				id = stateif.size()-1;
				break;

			case STATE_OUT:
				stateo.push_back(g);
				id = stateo.size()-1;
				break;

			case STATE_OUT_INIT:
				stateoi.push_back(g);
				id = stateoi.size()-1;
				break;

			case STATE_OUT_FINAL:
				stateof.push_back(g);
				id = stateof.size()-1;
				break;

			case PERSISTENT:
				persistent.push_back(g);
				id = persistent.size()-1;
				break;

			default
				break;
		}
		return id * (int)NUM_GATE_TYPE + (int)t;
	}

	shared_ptr<Gate> Circuit::get_gate(int id)
	{
		GateType t = (GateType)(id % (int)NUM_GATE_TYPE);
		int index = id / (int)NUM_GATE_TYPE;
		switch(t)
		{
			case GATE:
				return gate[index];

			case STREAM_IN:
				return streami[index];

			case STATE_IN:
				return statei[index];

			case STATE_IN_INIT:
				return stateii[index];

			case STATE_IN_FINAL:
				return stateif[index];

			case STATE_OUT:
				return stateo[index];

			case STATE_OUT_INIT:
				return stateoi[index];

			case STATE_OUT_FINAL:
				return stateof[index];

			case PERSISTENT:
				return persistent[index];

			default
				return nullptr;
		}
	}

	Port Circuit::get_state_out()
	{
		Port out;

		for (int i=0; i<stateoi.size(); i++)
		{
			out.init.push_back(stateoi[i]->output(CMB));
		}

		for (int i=0; i<stateo.size(); i++)
		{
			out.media.push_back(stateo[i]->output(CMB));
		}

		for (int i=0; i<stateof.size(); i++)
		{
			out.fin.push_back(stateof[i]->output(CMB));
		}

		return out;
	}

	void Circuit::set_state_in(Port states)
	{
		for (int i=0; i<states.init.size(); i++)
		{
			stateii[i]->set_value(states.init[i]);
		}

		for (int i=0; i<states.media.size(); i++)
		{
			statei[i]->set_value(states.media[i]);
		}

		for (int i=0; i<states.fin.size(); i++)
		{
			stateif[i]->set_value(states.fin[i]);
		}
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
		for (int i=0; i<stateii.size(); i++)
			stateii[i]->reset();
		for (int i=0; i<statei.size(); i++)
			statei[i]->reset();
		for (int i=0; i<stateif.size(); i++)
			stateif[i]->reset();
		for (int i=0; i<gate.size(); i++)
			gate[i]->reset();
		for (int i=0; i<stateoi.size(); i++)
			stateoi[i]->reset();
		for (int i=0; i<stateo.size(); i++)
			stateo[i]->reset();
		for (int i=0; i<stateof.size(); i++)
			stateof[i]->reset();
		for (int i=0; i<persistent.size(); i++)
			persistent[i]->negedge();
	}

	void Circuit::tick()
	{
		for (int i=0; i<streami.size(); i++)
			streami[i]->posedge();
		for (int i=0; i<stateii.size(); i++)
			stateii[i]->posedge();
		for (int i=0; i<statei.size(); i++)
			statei[i]->posedge();
		for (int i=0; i<stateif.size(); i++)
			stateif[i]->posedge();
		for (int i=0; i<gate.size(); i++)
			gate[i]->posedge();
		for (int i=0; i<stateoi.size(); i++)
			stateoi[i]->posedge();
		for (int i=0; i<stateo.size(); i++)
			stateo[i]->posedge();
		for (int i=0; i<stateof.size(); i++)
			stateof[i]->posedge();
		for (int i=0; i<persistent.size(); i++)
			persistent[i]->posedge();
	}

	int Circuit::combine_char(shared_ptr<Circuit> c, CombineType t)
	{
		switch(t)
		{
			case UNION:
			return combine_char_union(c);

			case PARALLEL:
			return combine_char_parallel(c);

			case STAR:
			return combine_char_star();

			case CONCATENATION:
			return combine_char_concatenation(c);

			default:
			return -1;
		}
	}

	int Circuit::combine_epsilon(shared_ptr<Circuit> c, CombineType t)
	{
		switch(t)
		{
			case UNION:
			return combine_epsilon_union(c);

			case PARALLEL:
			return combine_epsilon_parallel(c);

			case STAR:
			return combine_epsilon_star();

			case CONCATENATION:
			return combine_epsilon_concatenation(c);

			default:
			return -1;
		}
	}

	int Circuit::combine_char_parallel(shared_ptr<Circuit> c)
	{
		for (int i=0; i<c->stateii.size(); i++)
			stateii.push_back(c->stateii[i]);
		for (int i=0; i<c->stateif.size(); i++)
			stateif.push_back(c->stateif[i]);
		for (int i=0; i<c->statei.size(); i++)
			statei.push_back(c->statei[i]);
		for (int i=0; i<c->stateoi.size(); i++)
			stateoi.push_back(c->stateoi[i]);
		for (int i=0; i<c->stateof.size(); i++)
			stateof.push_back(c->stateof[i]);
		for (int i=0; i<c->stateo.size(); i++)
			stateo.push_back(c->stateo[i]);
		for (int i=0; i<c->streami.size(); i++)
			streami.push_back(c->streami[i]);
		for (int i=0; i<c->gate.size(); i++)
			gate.push_back(c->gate[i]);
		return 0;
	}

	int Circuit::combine_char_union(shared_ptr<Circuit> c)
	{
		return combine_char_parallel(c);
	}

	int Circuit::combine_char_concatenation(shared_ptr<Circuit> c)
	{
		return combine_char_parallel(c);
	}

	int Circuit::combine_char_star()
	{
		return 0;
	}

	int Circuit::combine_epsilon_parallel(shared_ptr<Circuit> c)
	{
		int l = stateii.size();
		combine_char_parallel(c);
		for (int i=l-1; i>=0; i--)
		{
			stateii[i+l].set_op(shared_ptr<CopyOp>(new CopyOp()));
			stateii[i+l].wire_in(Wire(stateii[i],CMB));
			stateii[i].wire_out(Wire(stateii[i+l],CMB));
			gate.push_back(stateii[i+l]);
			stateii.pop_back();
		}
	}

	int Circuit::combine_epsilon_union(shared_ptr<Circuit> c)
	{
		int l = stateof.size();
		combine_epsilon_parallel(c);
		for (int i=0; i<l; i++)
		{
			gate.push_back(stateof[i]);
			gate.push_back(stateof[i+l]);

			shared_ptr<Gate> tmp = shared_ptr<Gate>(new Gate(UNDEF,shared_ptr<UnionOp>(new UnionOp())));
			tmp->wire_in(Wire(stateof[i],CMB));
			tmp->wire_in(Wire(stateof[i+l],CMB));
			stateof[i]->wire_out(Wire(tmp,CMB));
			stateof[i+l]->wire_out(Wire(tmp,CMB));

			stateof[i] = tmp;
		}

		for (int i=0; i<l; i++)
			stateof.pop_back();

		return 0;
	}

	int Circuit::combine_epsilon_concatenation(shared_ptr<Circuit> c)
	{
		int l = stateii.size();
		combine_char_parallel(c);
		for (int i=0; i<l; i++)
		{
			stateii[i+l]->wire_in(stateof[i]);
			stateof[i]->wire_out(stateii[i+l]);
			stateii[i+l]->set_op(shared_ptr<CopyOp>(new CopyOp));

			gate.push_back(stateii[i+l]);
			gate.push_back(stateof[i]);
			stateof[i] = stateof[i+l];
		}

		for (int i=0; i<l; i++)
		{
			stateii.pop_back();
			stateof.pop_back();
		}
	}

	int Circuit::combine_epsilon_star()
	{
		for (int i=0; i<stateii.size(); i++)
		{
			shared_ptr<Gate> new_ii = shared_ptr<Gate>(new Gate(UNDEF,shared_ptr<ConstOp>(new ConstOp)));
			shared_ptr<Gate> new_of = shared_ptr<Gate>(new Gate(UNDEF,shared_ptr<CopyOp>(new CopyOp)));
			shared_ptr<Gate> old_ii = stateii[i];
			shared_ptr<Gate> old_of = stateof[i];

			old_ii->set_op(shared_ptr<UnionOp>(new UnionOp));
			old_ii->wire_in(Wire(old_of,SEQ));
			old_of->wire_out(Wire(old_ii,SEQ));
			old_ii->wire_in(Wire(new_ii,CMB));
			new_ii->wire_out(Wire(old_ii,CMB));
			new_of->wire_in(Wire(old_of,CMB));
			old_of->wire_out(Wire(new_of,CMB));

			persistent.push_back(old_of);
			stateof[i] = new_of;
			gate.push_back(old_ii);
			stateii[i] = new_ii;
		}
	}

}
