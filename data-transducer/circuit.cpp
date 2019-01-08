#include "circuit.h"

using std::shared_ptr;
using std::unique_ptr;
using std::vector;

namespace DT {

	Port::Port()
	{
	}

	Port::Port(const unique_ptr<Port> &src)
	{
		for (int i=0; i<src->init.size(); i++)
			init.push_back(copy_data(src->init[i]));
		for (int i=0; i<src->media.size(); i++)
			media.push_back(copy_data(src->media[i]));
		for (int i=0; i<src->fin.size(); i++)
			fin.push_back(copy_data(src->fin[i]));
	}

	void Port::merge(const unique_ptr<Port> &src, shared_ptr<MergeParallelOp> op)
	{
		to_param [] (const unique_ptr<DataValue> &a, const unique_ptr<DataValue> &b) -> vector<unique_ptr<DataValue> >
		{
			vector<unique_ptr<DataValue> > param;
			param.push_back(a);
			param.push_back(b);
			return param;
		}

		for (int i=0; i<src->init.size(); i++)
			init[i] = op(to_param(init[i],src->init[i]), nullptr);
		for (int i=0; i<src->media.size(); i++)
			media[i] = op(to_param(media[i],src->media[i]), nullptr);
		for (int i=0; i<src->fin.size(); i++)
			fin[i] = op(to_param(fin[i],src->fin[i]), nullptr);
	}

	Circuit::Circuit()
	{

	}

	void Circuit::add_gate(shared_ptr<Gate> g, GateType t)
	{
		gates.push_back(g);
		switch (t)
		{
			case GATE:
				break;

			case STREAM_IN:
				streami.push_back(g);
				break;

			case STATE_IN:
				statei.push_back(g);
				break;

			case STATE_IN_INIT:
				stateii.push_back(g);
				break;

			case STATE_IN_FINAL:
				stateif.push_back(g);
				break;

			case STATE_OUT:
				stateo.push_back(g);
				break;

			case STATE_OUT_INIT:
				stateoi.push_back(g);
				break;

			case STATE_OUT_FINAL:
				stateof.push_back(g);
				break;

			default
				break;
		}
	}

	unique_ptr<Port> Circuit::get_state_out()
	{
		unique_ptr<Port> out = unique_ptr<Port>(new Port());;

		for (int i=0; i<stateoi.size(); i++)
		{
			out->init.push_back(stateoi[i]->output());
		}

		for (int i=0; i<stateo.size(); i++)
		{
			out->media.push_back(stateo[i]->output());
		}

		for (int i=0; i<stateof.size(); i++)
		{
			out->fin.push_back(stateof[i]->output());
		}

		return out;
	}

	void Circuit::set_state_in(const unique_ptr<Port> &states)
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

	void Circuit::set_stream_in(const unique_ptr<DataValue> &val)
	{
		for (int i=0; i<streami.size(); i++)
		{
			streami[i]->set_value(val);
		}
	}

	void Circuit::reset()
	{
		for (int i=0; i<gates.size(); i++)
			gates[i]->reset();
	}

	void Circuit::tick()
	{
			gates[i]->posedge();
	}

	void Circuit::combine_char(shared_ptr<Circuit> c, CombineType t, share_ptr<PipelineOp> init_op, std::shared_ptr<MergeParallelOp> commit_op)
	{
		switch(t)
		{
			case UNION:
			return combine_char_union(c, init_op, commit_op);

			case PARALLEL:
			return combine_char_parallel(c,init_op, commit_op);

			case STAR:
			return combine_char_star(init_op, commit_op);

			case CONCATENATION:
			return combine_char_concatenation(init_op, commit_op);

			default:
			return -1;
		}
	}

	void Circuit::combine_epsilon(shared_ptr<Circuit> c, CombineType t, share_ptr<PipelineOp> init_op, std::shared_ptr<MergeParallelOp> op)
	{
		switch(t)
		{
			case UNION:
			return combine_epsilon_union(c, init_op, commit_op);

			case PARALLEL:
			return combine_epsilon_parallel(c, init_op, commit_op);

			case STAR:
			return combine_epsilon_star(init_op, commit_op);

			case CONCATENATION:
			return combine_epsilon_concatenation(c, init_op, commit_op);

			default:
			return -1;
		}
	}

	void Circuit::combine_basic(shared_ptr<Circuit> c, shared_ptr<PipelineOp> init_op, shared_ptr<MergeParallelOp> commit_op) 
	{
		for (int i=0; i<c->statei.size(); i++)
			statei.push_back(c->statei[i]);
		for (int i=0; i<c->stateo.size(); i++)
			stateo.push_back(c->stateo[i]);
		for (int i=0; i<c->streami.size(); i++)
			streami.push_back(c->streami[i]);
		for (int i=0; i<c->gates.size(); i++)
			gates.push_back(c->gates[i]);
	}

	void Circuit::combine_char_parallel(shared_ptr<Circuit> c, shared_ptr<PipelineOp> init_op, shared_ptr<MergeParallelOp> commit_op)
	{
		combine_char_union(c);
	}

	void Circuit::combine_epsilon_parallel(shared_ptr<Circuit> c, shared_ptr<PipelineOp> init_op, shared_ptr<MergeParallelOp> commit_op)
	{
		int l = stateof.size();
		combine_basic(c);
		for (int i=0; i<l; i++)
		{
			shared_ptr<Gate> new_of = shared_ptr<Gate>(new Gate(commit_op));
			shared_ptr<Gate> new_if = shared_ptr<Gate>(new Gate(shared_ptr<ConstOp>(new ConstOp())));
			new_of->wire_in(new_if);
			new_if->wire_out(new_of);

			statei.push_back(stateif[i]);
			statei.push_back(c->stateii[i]);
			statei.push_back(c->stateif[i]);
			stateo.push_back(stateof[i]);
			stateo.push_back(c->stateoi[i]);
			stateo.push_back(c->stateof[i]);

			stateof[i] = new_of;
			stateif[i] = new_if;
		}
	}

	void Circuit::combine_char_union(shared_ptr<Circuit> c, shared_ptr<PipelineOp> init_op, shared_ptr<MergeParallelOp> commit_op)
	{
		int l = stateof.size();
		combine_basic(c);
		for (int i=0; i<l; i++)
		{
			shared_ptr<Gate> new_of = shared_ptr<Gate>(new Gate(shared_ptr<UnionOp>(new CopyOp())));
			shared_ptr<Gate> new_if = shared_ptr<Gate>(new Gate(shared_ptr<ConstOp>(new ConstOp())));
			new_of->wire_in(new_if);
			new_if->wire_out(new_of);

			statei.push_back(stateif[i]);
			statei.push_back(c->stateii[i]);
			statei.push_back(c->stateif[i]);
			stateo.push_back(stateof[i]);
			stateo.push_back(c->stateoi[i]);
			stateo.push_back(c->stateof[i]);

			stateof[i] = new_of;
			stateif[i] = new_if;
		}
	}

	void Circuit::combine_epsilon_union(shared_ptr<Circuit> c, shared_ptr<PipelineOp> init_op, shared_ptr<MergeParallelOp> commit_op)
	{
		int l = stateof.size();
		combine_basic(c);
		for (int i=0; i<l; i++)
		{
			shared_ptr<Gate> new_of = shared_ptr<Gate>(new Gate(shared_ptr<UnionOp>(new UnionOp())));
			shared_ptr<Gate> new_if = shared_ptr<Gate>(new Gate(shared_ptr<ConstOp>(new ConstOp())));
			new_of->wire_in(stateof[i]);
			new_of->wire_in(c->stateof[i]);
			stateof[i]->wire_out(new_of);
			c->stateof[i]->wire_out(new_of);
			stateii[i]->wire_out(c->stateii[i]);
			c->stateii[i]->wire_in(stateii[i]);

			statei.push_back(stateif[i]);
			statei.push_back(c->stateii[i]);
			statei.push_back(c->stateif[i]);
			stateo.push_back(stateof[i]);
			stateo.push_back(c->stateoi[i]);
			stateo.push_back(c->stateof[i]);

			stateof[i] = new_of;
			stateif[i] = new_if;
		}
	}

	void Circuit::combine_char_concatenation(shared_ptr<Circuit> c, shared_ptr<PipelineOp> init_op, shared_ptr<MergeParallelOp> commit_op)
	{
		int l = stateii.size();
		combine_basic(c);

		for (int i=0; i<l; i++)
		{
			statei.push_back(stateif[i]);
			statei.push_back(c->stateii[i]);
			stateo.push_back(stateof[i]);
			stateo.push_back(c->stateoi[i]);

			stateif[i] = c->stateif[i];
			stateof[i] = c->stateof[i];
		}
	}


	void Circuit::combine_epsilon_concatenation(shared_ptr<Circuit> c, shared_ptr<PipelineOp> init_op, shared_ptr<MergeParallelOp> commit_op)
	{
		int l = stateii.size();
		combine_basic(c);

		for (int i=0; i<l; i++)
		{
			c->stateii[i]->wire_in(stateof[i]);
			stateof[i]->wire_out(c->stateii[i]);
			c->stateii[i]->set_op(shared_ptr<CopyOp>(new CopyOp));

			statei.push_back(stateif[i]);
			statei.push_back(c->stateii[i]);
			stateo.push_back(stateof[i]);
			stateo.push_back(c->stateoi[i]);

			stateif[i] = c->stateif[i];
			stateof[i] = c->stateof[i];
		}

	}

	void Circuit::combine_char_star(shared_ptr<PipelineOp> init_op, shared_ptr<MergeParallelOp> commit_op)
	{
		vector< std::shared_ptr<Gate> > new_stateii; /* new input state */
		vector< std::shared_ptr<Gate> > new_stateif; /* new output state */
		vector< std::shared_ptr<Gate> > new_stateoi; /* new input state */
		vector< std::shared_ptr<Gate> > new_stateof; /* new output state */
		for (int i=0; i<stateii.size(); i++)
		{
			shared_ptr<Gate> new_ii = shared_ptr<Gate>(new Gate(shared_ptr<ConstOp>(new ConstOp())));
			shared_ptr<Gate> new_oi = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp())));
			shared_ptr<Gate> new_if = shared_ptr<Gate>(new Gate(shared_ptr<ConstOp>(new ConstOp())));
			shared_ptr<Gate> new_of = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp())));
			shared_ptr<Gate> old_ii = stateii[i];
			shared_ptr<Gate> old_oi = stateoi[i];
			shared_ptr<Gate> old_if = stateif[i];
			shared_ptr<Gate> old_of = stateof[i];

			new_ii->wire_out(new_oi);
			new_oi->wire_in(new_ii);
			new_if->wire_out(new_of);
			new_of->wire_in(new_if);

			statei.push_back(old_ii);
			statei.push_back(old_if);
			stateo.push_back(old_oi);
			stateo.push_back(old_of);
			new_stateii.push_back(new_ii);
			new_stateif.push_back(new_if);
			new_stateoi.push_back(new_oi);
			new_stateof.push_back(new_of);
			gates.push_back(new_ii);
			gates.push_back(new_oi);
			gates.push_back(new_if);
			gates.push_back(new_of);
		}
		stateii = new_stateii;
		stateif = new_stateif;
		stateoi = new_stateoi;
		stateof = new_stateof;
	}

	void Circuit::combine_epsilon_star(shared_ptr<PipelineOp> init_op, shared_ptr<MergeParallelOp> commit_op)
	{
		vector< std::shared_ptr<Gate> > new_stateii; /* new input state */
		vector< std::shared_ptr<Gate> > new_stateif; /* new output state */
		vector< std::shared_ptr<Gate> > new_stateoi; /* new input state */
		vector< std::shared_ptr<Gate> > new_stateof; /* new output state */
		for (int i=0; i<stateii.size(); i++)
		{
			shared_ptr<Gate> new_ii = shared_ptr<Gate>(new Gate(init_op));
			shared_ptr<Gate> new_oi = shared_ptr<Gate>(new Gate(shared_ptr<ConstOp>(new ConstOp())));
			shared_ptr<Gate> new_if = shared_ptr<Gate>(new Gate(shared_ptr<ConstOp>(new ConstOp())));
			shared_ptr<Gate> new_of = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp())));
			shared_ptr<Gate> old_ii = stateii[i];
			shared_ptr<Gate> old_oi = stateoi[i];
			shared_ptr<Gate> old_if = stateif[i];
			shared_ptr<Gate> old_of = stateof[i];

			old_ii->set_op(commit_op);
			old_ii->wire_in(old_if);
			old_if->wire_out(old_ii);
			old_ii->wire_in(new_ii);
			new_ii->wire_out(old_ii);
			new_of->wire_in(old_ii);
			old_ii->wire_out(new_of);

			statei.push_back(old_ii);
			statei.push_back(old_if);
			stateo.push_back(old_oi);
			stateo.push_back(old_of);
			new_stateii.push_back(new_ii);
			new_stateif.push_back(new_if);
			new_stateoi.push_back(new_oi);
			new_stateof.push_back(new_of);
			gates.push_back(new_ii);
			gates.push_back(new_oi);
			gates.push_back(new_if);
			gates.push_back(new_of);
		}
		stateii = new_stateii;
		stateif = new_stateif;
		stateoi = new_stateoi;
		stateof = new_stateof;
	}

}

std::shared_ptr<Circuit> Circuit::get_plain_circuit()
{
	auto ans = shared_ptr<Circuit>(new Circuit());

	if (stateii.size() != stateoi.size())
		return nullptr;
	if (stateif.size() != stateof.size())
		return nullptr;
	if (statei.size() != stateo.size())
		return nullptr;

	/* streami is empty */

	for (int i=0; i<statei.size(); i++)
	{
		auto tmpi = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp())));
		auto tmpo = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp())));
		tmpi->wire_out(tmpo);
		tmpo->wire_in(tmpi);
		ans->statei.push_back(tmpi);
		ans->stateo.push_back(tmpo);
		ans->gates.push_back(tmpi);
		ans->gates.push_back(tmpo);
	}


	for (int i=0; i<stateii.size(); i++)
	{
		auto tmpi = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp())));
		auto tmpo = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp())));
		tmpi->wire_out(tmpo);
		tmpo->wire_in(tmpi);
		ans->stateii.push_back(tmpi);
		ans->stateoi.push_back(tmpo);
		ans->gates.push_back(tmpi);
		ans->gates.push_back(tmpo);
	}


	for (int i=0; i<stateif.size(); i++)
	{
		auto tmpi = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp())));
		auto tmpo = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp())));
		tmpi->wire_out(tmpo);
		tmpo->wire_in(tmpi);
		ans->stateif.push_back(tmpi);
		ans->stateof.push_back(tmpo);
		ans->gates.push_back(tmpi);
		ans->gates.push_back(tmpo);
	}

	return ans;
}
