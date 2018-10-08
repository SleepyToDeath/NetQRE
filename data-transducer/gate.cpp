#include "gate.h"

namespace DT {

Gate::Gate(int init, shared_ptr<Op> op)
{
	val = init;
	val_old = init;
	cmb_wires = 0;
	ready_wires = 0;
	this->op = op;
}

Gate::~Gate()
{
}

void Gate::wire_in(Wire w)
{
	in.push_back(w);
	if (w->t == CMB)
		cmb_wires ++;
}

void Gate::wire_out(Wire w)
{
	out.push_back(w);
}

void Gate::wire_ready()
{
	ready_wires ++;
}

void Gate::posedge()
{
	if (ready_wires == cmb_wires)
	{
		{
			std::vector<int> param;
			for (int i=0; i<in.size(); i++)
			{
				param.push_back( in[i].g->output( in[i].t ) );
			}
			val = (shared_ptr<op>)(param, val);
		}
		
		for (int i=0; i<out.size(); i++)
			if (out[i].t == CMB)
			{
				out[i].g->wire_ready();
				out[i].g->posedge();
			}

		ready_wires ++;
	}
}

void Gate::negedge()
{
	ready_wires = 0;
	val_old = val;
}

void reset()
{
	ready_wires = 0;
	val = init;
	val_old = init;
}

int Gate::output(WireType t)
{
	if (CMB == t)
		return val;
	else
		return val_old;
}

void Gate::set_value(int val)
{
	this->val = val;
}

void Gate::set_op(shared_ptr<Op> op)
{
	delete this->op;
	this->op = op;
}

}
