#include "gate.h"

using std::move();

namespace DT {

Gate(shared_ptr<Gate> src)
:Gate(src->op)
{
}

Gate(shared_ptr<Op> op)
{
	val = DataValue::factory->get_instance(UNDEF);
	val_old = copy_data(val);
	val_init = copy_data(val);
	cmb_wires = 0;
	ready_wires = 0;
	this->op = op;
}

Gate::~Gate()
{
}

void Gate::wire_in(share_ptr<Gate> src)
{
	in.push_back(src);
	cmb_wires ++;
}

void Gate::wire_out(share_ptr<Gate> dst)
{
	out.push_back(dst);
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
			std::vector<unique_ptr<DataValue> > param;
			for (int i=0; i<in.size(); i++)
			{
				param.push_back( in[i].g->output( in[i].t ) );
			}
			val = (*shared_ptr<op>)(param, val);
		}
		
		for (int i=0; i<out.size(); i++)
		{
			out[i]->wire_ready();
			out[i]->posedge();
		}

		ready_wires ++; /* only posedge once */
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
	val = copy_data(val_init);
	val_old = copy_data(val_init);
}

unique_ptr<DataValue> Gate::output()
{
	return copy_data(val);
}

void Gate::set_value(unique_ptr<DataValue> val)
{
	this->val = copy_data(val);
}

void Gate::set_op(shared_ptr<Op> op)
{
	this->op = op;
}

}
