#include "circuit.h"
#include <iostream>

using std::shared_ptr;
using std::unique_ptr;
using std::vector;
using std::string;
using std::move;

using std::cout;
using std::endl;

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
		auto to_param = [] (const unique_ptr<DataValue> &a, const unique_ptr<DataValue> &b) -> vector<unique_ptr<DataValue> >
		{
			vector<unique_ptr<DataValue> > param;
			param.push_back(copy_data(a));
			param.push_back(copy_data(b));
			return move(param);
		};

		cout<<"Merging ports:\n";
		for (int i=0; i<src->init.size(); i++)
		{
			if (init[i]->type == VALID)
				cout<<"init"<<i<<" "<<init[i]->to_string()<<" -> ";
			init[i] = (*op)(to_param(init[i],src->init[i]), nullptr);
			if (init[i]->type == VALID)
				cout<<init[i]->to_string()<<endl;
		}
		for (int i=0; i<src->media.size(); i++)
		{
			if (media[i]->type == VALID)
				cout<<"med"<<i<<" "<<media[i]->to_string()<<" -> ";
			media[i] = (*op)(to_param(media[i],src->media[i]), nullptr);
			if (media[i]->type == VALID)
				cout<<media[i]->to_string()<<endl;
		}
		for (int i=0; i<src->fin.size(); i++)
		{
			if (fin[i]->type == VALID)
				cout<<"fin"<<i<<" "<<fin[i]->to_string()<<" -> ";
			fin[i] = (*op)(to_param(fin[i],src->fin[i]), nullptr);
			if (fin[i]->type == VALID)
				cout<<fin[i]->to_string()<<endl;
		}
	}

	Circuit::Circuit()
	{

	}

	void Circuit::add_gate(shared_ptr<Gate> g, GateType t)
	{
		gates.push_back(g);
		switch (t)
		{
			case GateType::GATE:
				break;

			case GateType::STREAM_IN:
				streami.push_back(g);
				break;

			case GateType::STATE_IN:
				statei.push_back(g);
				break;

			case GateType::STATE_IN_INIT:
				stateii.push_back(g);
				break;

			case GateType::STATE_IN_FINAL:
				stateif.push_back(g);
				break;

			case GateType::STATE_OUT:
				stateo.push_back(g);
				break;

			case GateType::STATE_OUT_INIT:
				stateoi.push_back(g);
				break;

			case GateType::STATE_OUT_FINAL:
				stateof.push_back(g);
				break;

			default:
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
		for (int i=0; i<states->init.size(); i++)
		{
			stateii[i]->set_value(states->init[i]);
		}

		for (int i=0; i<states->media.size(); i++)
		{
			statei[i]->set_value(states->media[i]);
			stateo[i]->set_value(states->media[i]);
		}

		for (int i=0; i<states->fin.size(); i++)
		{
			stateof[i]->set_value(states->fin[i]);
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
		cout<<endl;
	}

	void Circuit::tick()
	{
		/* debug */
		for (int i=0; i<statei.size(); i++)
			statei[i]->id = i;
		for (int i=0; i<stateo.size(); i++)
			stateo[i]->id = i;
		for (int i=0; i<stateii.size(); i++)
			stateii[i]->id = i;
		for (int i=0; i<stateif.size(); i++)
			stateif[i]->id = i;
		for (int i=0; i<stateoi.size(); i++)
			stateoi[i]->id = i;
		for (int i=0; i<stateof.size(); i++)
			stateof[i]->id = i;
		for (int i=0; i<streami.size(); i++)
			streami[i]->id = i;


		for (int i=0; i<gates.size(); i++)
			gates[i]->posedge();
	}

	void Circuit::combine_char(shared_ptr<Circuit> c, CombineType t)
	{
		switch(t)
		{
			case CombineType::UNION:
			return combine_char_union(c);

			case CombineType::PARALLEL:
			combine_char_parallel(c);
			return;

			case CombineType::STAR:
			combine_char_star();
			return;

			case CombineType::CONCATENATION:
			combine_char_concatenation(c);
			return;

			case CombineType::CONDITIONAL:
			combine_char_conditional();
			return;

			default:
			throw string("Unknown combine type!\n");
		}
	}

	void Circuit::combine_epsilon(shared_ptr<Circuit> c, CombineType t, shared_ptr<MergeParallelOp> merge_op, shared_ptr<PipelineOp> init_op, std::shared_ptr<PipelineOp> commit_op)
	{
		switch(t)
		{
			case CombineType::UNION:
			cout<<"Combine Type: Union\n";
			combine_epsilon_union(c, init_op, commit_op);
			return;

			case CombineType::PARALLEL:
			cout<<"Combine Type: Parallel\n";
			combine_epsilon_parallel(c, init_op, commit_op);
			return;

			case CombineType::STAR:
			cout<<"Combine Type: Star\n";
			combine_epsilon_star(merge_op, init_op, commit_op);
			return;

			case CombineType::CONCATENATION:
			cout<<"Combine Type: Concatenation\n";
			combine_epsilon_concatenation(c, init_op, commit_op);
			return;

			case CombineType::CONDITIONAL:
			cout<<"Combine Type: Conditional\n";
			combine_epsilon_conditional(commit_op);
			return;

			default:
			throw string("Unknown combine type!\n");
		}
	}

	void Circuit::combine_basic(shared_ptr<Circuit> c)
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

	void Circuit::combine_char_parallel(shared_ptr<Circuit> c)
	{
		combine_basic(c);
		/* [TODO] not used now */
	}

	void Circuit::combine_epsilon_parallel(shared_ptr<Circuit> c, shared_ptr<PipelineOp> init_op, shared_ptr<PipelineOp> commit_op)
	{
		int l = stateof.size();
		combine_basic(c);
		/* [TODO] not used now */
		/*
		for (int i=0; i<l; i++)
		{
			auto new_of = shared_ptr<Gate>(new Gate(commit_op));
			auto new_if = shared_ptr<Gate>(new Gate(shared_ptr<ConstOp>(new ConstOp())));
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
		*/
	}

	void Circuit::combine_char_union(shared_ptr<Circuit> c)
	{
		int l = stateof.size();
		combine_basic(c);
		/* [TODO] not used now */
		/*
		for (int i=0; i<l; i++)
		{
			auto new_of = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp())));
			auto new_if = shared_ptr<Gate>(new Gate(shared_ptr<ConstOp>(new ConstOp())));
			new_of->wire_in(new_if);
			new_if->wire_out(new_of);

			statei.push_back(stateif[i]);
			statei.push_back(c->stateii[i]);
			statei.push_back(c->stateif[i]);
			stateo.push_back(stateof[i]);
			stateo.push_back(c->stateoi[i]);
			stateo.push_back(c->stateof[i]);
			gates.push_back(new_of);
			gates.push_back(new_if);

			stateof[i] = new_of;
			stateif[i] = new_if;
		}
		*/
	}

	void Circuit::combine_epsilon_union(shared_ptr<Circuit> c, shared_ptr<PipelineOp> init_op, shared_ptr<PipelineOp> commit_op)
	{
		int l = stateof.size();
		combine_basic(c);
		/* [TODO] not used now */
		/*
		for (int i=0; i<l; i++)
		{
			auto new_of = shared_ptr<Gate>(new Gate(shared_ptr<UnionOp>(new UnionOp())));
			auto new_if = shared_ptr<Gate>(new Gate(shared_ptr<ConstOp>(new ConstOp())));
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
		*/
	}

	void Circuit::combine_char_concatenation(shared_ptr<Circuit> c)
	{
		int l = stateii.size();
		combine_basic(c);

		vector< std::shared_ptr<Gate> > new_stateii; /* new input state */
		vector< std::shared_ptr<Gate> > new_stateif; /* new output state */
		vector< std::shared_ptr<Gate> > new_stateoi; /* new input state */
		vector< std::shared_ptr<Gate> > new_stateof; /* new output state */

		for (int i=0; i<l; i++)
		{
			auto const_op = shared_ptr<ConstOp>(new ConstOp());
			auto copy_op = shared_ptr<CopyOp>(new CopyOp());
			auto init_i = shared_ptr<Gate>(new Gate(const_op, "ccinit_i"));
			auto init_o = shared_ptr<Gate>(new Gate(copy_op, "ccinit_o"));
			auto commit_li = shared_ptr<Gate>(new Gate(const_op, "cccommit_li"));
			auto commit_lo = shared_ptr<Gate>(new Gate(copy_op, "cccommit_lo"));
			auto commit_ri = shared_ptr<Gate>(new Gate(const_op, "cccommit_ri"));
			auto commit_ro = shared_ptr<Gate>(new Gate(copy_op, "cccommit_ro"));

			auto lii = stateii[i];
			auto loi = stateoi[i];
			auto lif = stateif[i];
			auto lof = stateof[i];
			auto rii = c->stateii[i];
			auto roi = c->stateoi[i];
			auto rif = c->stateif[i];
			auto rof = c->stateof[i];

			init_i->wire_out(init_o);
			commit_li->wire_out(commit_lo);
			commit_ri->wire_out(commit_ro);

			init_o->wire_in(init_i);
			commit_lo->wire_in(commit_li);
			commit_ro->wire_in(commit_ri);

			new_stateii.push_back(init_i);
			new_stateoi.push_back(init_o);
			new_stateif.push_back(commit_ri);
			new_stateof.push_back(commit_ro);

			statei.push_back(lii);
			statei.push_back(lif);
			stateo.push_back(loi);
			stateo.push_back(lof);
			statei.push_back(rii);
			statei.push_back(rif);
			stateo.push_back(roi);
			stateo.push_back(rof);
			statei.push_back(commit_li);
			stateo.push_back(commit_lo);

			gates.push_back(init_i);
			gates.push_back(init_o);
			gates.push_back(commit_li);
			gates.push_back(commit_lo);
			gates.push_back(commit_ri);
			gates.push_back(commit_ro);
		}

		stateii = new_stateii;
		stateif = new_stateif;
		stateoi = new_stateoi;
		stateof = new_stateof;

		if (!check())
			throw string("Incorrect combine cc!\n");
		tick();
		reset();
	}


	void Circuit::combine_epsilon_concatenation(shared_ptr<Circuit> c, shared_ptr<PipelineOp> init_op, shared_ptr<PipelineOp> commit_op)
	{
		int l = stateii.size();
		combine_basic(c);

		vector< std::shared_ptr<Gate> > new_stateii; /* new input state */
		vector< std::shared_ptr<Gate> > new_stateif; /* new output state */
		vector< std::shared_ptr<Gate> > new_stateoi; /* new input state */
		vector< std::shared_ptr<Gate> > new_stateof; /* new output state */
	
		for (int i=0; i<l; i++)
		{
			auto const_op = shared_ptr<ConstOp>(new ConstOp());
			auto init_i = shared_ptr<Gate>(new Gate(init_op, "ecinit_i"));
			auto init_o = shared_ptr<Gate>(new Gate(const_op, "ecinit_o"));
			auto commit_li = shared_ptr<Gate>(new Gate(const_op, "commit_li"));
			auto commit_lo = shared_ptr<Gate>(new Gate(commit_op, "commit_lo"));
			auto commit_ri = shared_ptr<Gate>(new Gate(const_op, "commit_ri"));
			auto commit_ro = shared_ptr<Gate>(new Gate(commit_op, "commit_ro"));

			auto lii = stateii[i];
			auto loi = stateoi[i];
			auto lif = stateif[i];
			auto lof = stateof[i];
			auto rii = c->stateii[i];
			auto roi = c->stateoi[i];
			auto rif = c->stateif[i];
			auto rof = c->stateof[i];

			init_i->wire_out(lii);
			lof->wire_out(commit_lo);
			commit_lo->wire_out(rii);
			rof->wire_out(commit_ro);

			lii->wire_in(init_i);
			commit_lo->wire_in(lof);
			rii->wire_in(commit_lo);
			commit_ro->wire_in(rof);

			new_stateii.push_back(init_i);
			new_stateoi.push_back(init_o);
			new_stateif.push_back(commit_ri);
			new_stateof.push_back(commit_ro);

			statei.push_back(lii);
			statei.push_back(lif);
			stateo.push_back(loi);
			stateo.push_back(lof);
			statei.push_back(rii);
			statei.push_back(rif);
			stateo.push_back(roi);
			stateo.push_back(rof);
			statei.push_back(commit_li);
			stateo.push_back(commit_lo);

			gates.push_back(init_i);
			gates.push_back(init_o);
			gates.push_back(commit_li);
			gates.push_back(commit_lo);
			gates.push_back(commit_ri);
			gates.push_back(commit_ro);
		}

		stateii = new_stateii;
		stateif = new_stateif;
		stateoi = new_stateoi;
		stateof = new_stateof;

		if (!check())
			throw string("Incorrect combine ec!\n");
		tick();
		reset();
	}

	void Circuit::combine_char_star()
	{
		vector< std::shared_ptr<Gate> > new_stateii; /* new input state */
		vector< std::shared_ptr<Gate> > new_stateif; /* new output state */
		vector< std::shared_ptr<Gate> > new_stateoi; /* new input state */
		vector< std::shared_ptr<Gate> > new_stateof; /* new output state */
		for (int i=0; i<stateii.size(); i++)
		{
			auto const_op = shared_ptr<ConstOp>(new ConstOp());
			auto copy_op = shared_ptr<CopyOp>(new CopyOp());
			auto new_ii = shared_ptr<Gate>(new Gate(const_op, "csii"));
			auto new_oi = shared_ptr<Gate>(new Gate(copy_op, "csoi"));
			auto new_if = shared_ptr<Gate>(new Gate(const_op, "csif"));
			auto new_of = shared_ptr<Gate>(new Gate(copy_op, "csof"));
			auto commit_i = shared_ptr<Gate>(new Gate(const_op, "cscommit_i"));
			auto commit_o = shared_ptr<Gate>(new Gate(copy_op, "cscommit_o"));
			auto merge_i = shared_ptr<Gate>(new Gate(const_op, "csmerge_i"));
			auto merge_o = shared_ptr<Gate>(new Gate(copy_op, "csmerge_o"));
			auto old_ii = stateii[i];
			auto old_oi = stateoi[i];
			auto old_if = stateif[i];
			auto old_of = stateof[i];

			new_ii->wire_out(new_oi);
			new_oi->wire_in(new_ii);
			new_if->wire_out(new_of);
			new_of->wire_in(new_if);
			commit_i->wire_out(commit_o);
			commit_o->wire_in(commit_i);
			merge_i->wire_out(merge_o);
			merge_o->wire_in(merge_i);

			statei.push_back(old_ii);
			statei.push_back(old_if);
			stateo.push_back(old_oi);
			stateo.push_back(old_of);
			statei.push_back(commit_i);
			statei.push_back(merge_i);
			stateo.push_back(commit_o);
			stateo.push_back(merge_o);
			new_stateii.push_back(new_ii);
			new_stateif.push_back(new_if);
			new_stateoi.push_back(new_oi);
			new_stateof.push_back(new_of);
			gates.push_back(commit_i);
			gates.push_back(commit_o);
			gates.push_back(merge_i);
			gates.push_back(merge_o);
			gates.push_back(new_ii);
			gates.push_back(new_oi);
			gates.push_back(new_if);
			gates.push_back(new_of);
		}
		stateii = new_stateii;
		stateif = new_stateif;
		stateoi = new_stateoi;
		stateof = new_stateof;

		if (!check())
			throw string("Incorrect combine cs!\n");
		tick();
		reset();
	}

	void Circuit::combine_epsilon_star(shared_ptr<MergeParallelOp> merge_op, shared_ptr<PipelineOp> init_op, shared_ptr<PipelineOp> commit_op)
	{
		vector< std::shared_ptr<Gate> > new_stateii; /* new input state */
		vector< std::shared_ptr<Gate> > new_stateif; /* new output state */
		vector< std::shared_ptr<Gate> > new_stateoi; /* new input state */
		vector< std::shared_ptr<Gate> > new_stateof; /* new output state */
		for (int i=0; i<stateii.size(); i++)
		{
			auto const_op = shared_ptr<ConstOp>(new ConstOp());
			auto copy_op = shared_ptr<CopyOp>(new CopyOp());
			auto new_ii = shared_ptr<Gate>(new Gate(init_op, "esii"));
			auto new_oi = shared_ptr<Gate>(new Gate(const_op, "esoi"));
			auto new_if = shared_ptr<Gate>(new Gate(const_op, "esif"));
			auto new_of = shared_ptr<Gate>(new Gate(copy_op, "esof"));
			
			auto old_ii = stateii[i];
			auto old_oi = stateoi[i];
			auto old_if = stateif[i];
			auto old_of = stateof[i];
			auto commit_i = shared_ptr<Gate>(new Gate(commit_op, "escommit_i"));
			auto commit_o = shared_ptr<Gate>(new Gate(const_op, "escommit_o"));
			auto merge_i = shared_ptr<Gate>(new Gate(merge_op, "esmerge_i"));
			auto merge_o = shared_ptr<Gate>(new Gate(const_op, "esmerge_o"));

			commit_i->wire_in_seq(old_of);
			old_of->wire_out_seq(commit_i);
			merge_i->wire_in(commit_i);
			commit_i->wire_out(merge_i);
			merge_i->wire_in(new_ii);
			new_ii->wire_out(merge_i);
			old_ii->wire_in(merge_i);
			merge_i->wire_out(old_ii);
			new_of->wire_in(merge_i);
			merge_i->wire_out(new_of);

			statei.push_back(old_ii);
			statei.push_back(old_if);
			stateo.push_back(old_oi);
			stateo.push_back(old_of);
			statei.push_back(commit_i);
			statei.push_back(merge_i);
			stateo.push_back(commit_o);
			stateo.push_back(merge_o);
			new_stateii.push_back(new_ii);
			new_stateif.push_back(new_if);
			new_stateoi.push_back(new_oi);
			new_stateof.push_back(new_of);
			gates.push_back(commit_i);
			gates.push_back(commit_o);
			gates.push_back(merge_i);
			gates.push_back(merge_o);
			gates.push_back(new_ii);
			gates.push_back(new_oi);
			gates.push_back(new_if);
			gates.push_back(new_of);
		}
		stateii = new_stateii;
		stateif = new_stateif;
		stateoi = new_stateoi;
		stateof = new_stateof;

		if (!check())
			throw string("Incorrect combine es!\n");
		tick();
		reset();
	}

	void Circuit::combine_char_conditional() 
	{
		vector< std::shared_ptr<Gate> > new_stateif; /* new output state */
		vector< std::shared_ptr<Gate> > new_stateof; /* new output state */
		for (int i=0; i<stateii.size(); i++)
		{
			auto new_if = shared_ptr<Gate>(new Gate(shared_ptr<ConstOp>(new ConstOp()), "cccif"));
			auto new_of = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp()), "cccof"));
			auto old_if = stateif[i];
			auto old_of = stateof[i];
			new_if->wire_out(new_of);
			new_of->wire_in(new_if);
			new_stateif.push_back(new_if);
			new_stateof.push_back(new_of);
			statei.push_back(old_if);
			stateo.push_back(old_of);
			gates.push_back(new_if);
			gates.push_back(new_of);
		}
		stateif = new_stateif;
		stateof = new_stateof;

		if (!check())
			throw string("Incorrect combine ccc!\n");
		tick();
		reset();
	}

	void Circuit::combine_epsilon_conditional(shared_ptr<PipelineOp> commit_op)
	{
		vector< std::shared_ptr<Gate> > new_stateif; /* new output state */
		vector< std::shared_ptr<Gate> > new_stateof; /* new output state */
		for (int i=0; i<stateii.size(); i++)
		{
			auto new_if = shared_ptr<Gate>(new Gate(shared_ptr<ConstOp>(new ConstOp()), "eccif"));
			auto new_of = shared_ptr<Gate>(new Gate(commit_op, "eccof"));
			auto old_if = stateif[i];
			auto old_of = stateof[i];
			new_of->wire_in(old_of);
			old_of->wire_out(new_of);
			new_stateif.push_back(new_if);
			new_stateof.push_back(new_of);
			statei.push_back(old_if);
			stateo.push_back(old_of);
			gates.push_back(new_if);
			gates.push_back(new_of);
		}
		stateif = new_stateif;
		stateof = new_stateof;

		if (!check())
			throw string("Incorrect combine ecc!\n");
		tick();
		reset();
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

		for (int i=0; i<streami.size(); i++)
		{
			auto tmp = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp()), streami[i]->name));
			ans->streami.push_back(tmp);
			ans->gates.push_back(tmp);
		}

		for (int i=0; i<statei.size(); i++)
		{
			auto tmpi = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp()), statei[i]->name));
			auto tmpo = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp()), stateo[i]->name));
			tmpi->wire_out(tmpo);
			tmpo->wire_in(tmpi);
			ans->statei.push_back(tmpi);
			ans->stateo.push_back(tmpo);
			ans->gates.push_back(tmpi);
			ans->gates.push_back(tmpo);
		}


		for (int i=0; i<stateii.size(); i++)
		{
			auto tmpi = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp()), stateii[i]->name));
			auto tmpo = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp()), stateoi[i]->name));
			tmpi->wire_out(tmpo);
			tmpo->wire_in(tmpi);
			ans->stateii.push_back(tmpi);
			ans->stateoi.push_back(tmpo);
			ans->gates.push_back(tmpi);
			ans->gates.push_back(tmpo);
		}


		for (int i=0; i<stateif.size(); i++)
		{
			auto tmpi = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp()), stateif[i]->name));
			auto tmpo = shared_ptr<Gate>(new Gate(shared_ptr<CopyOp>(new CopyOp()), stateof[i]->name));
			tmpi->wire_out(tmpo);
			tmpo->wire_in(tmpi);
			ans->stateif.push_back(tmpi);
			ans->stateof.push_back(tmpo);
			ans->gates.push_back(tmpi);
			ans->gates.push_back(tmpo);
		}

		if (ans->size() != size())
		{
			cout<<ans->size()<<" : "<<size()<<endl;
			throw string("Incorrect circuit copy!\n");
		}

		ans->tick();
		ans->reset();

		return ans;
	}

	int Circuit::size()
	{
		return gates.size();
	}

	bool Circuit::check()
	{
		return statei.size() + stateo.size() + stateii.size() + stateif.size() + stateoi.size() + stateof.size() + streami.size() == gates.size();
	}
}
