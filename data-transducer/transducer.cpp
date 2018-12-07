#include "transducer.h"

namespace DT
{

Transducer::Transducer(int param_number, int tag_alphabet_size, std::shared_ptr<MergeParallelOp> state_merger);
{
	this->param_number = param_number;
	this->tag_alphabet_size = tag_alphabet_size;
	this->state_merger = state_merger;
}

Transducer::~Transducer()
{
}

void Transducer::add_circuit(shared_ptr<Circuit> c, TagType tag)
{
	circuits[tag] = c;
}

void Transducer::combine(shared_ptr<Transducer> dt, CombineType t)
{
	epsilon_circuit->combine_epsilon(dt->epsilon_circuit,t);
	for_each(circuits.begin(), circuits.end(), [](shared_ptr<circuits> c)
	{
		c->combine_char(dt->circuits[key],t);
	} )
}

int Transducer::init(std::vector<int> parameters)
{
	NullPort = copy_port(states);
	for (int i=0; i<NullPort->init.size(); i++)
		NullPort->init[i]->type = UNDEF;
	for (int i=0; i<NullPort->media.size(); i++)
		NullPort->media[i]->type = UNDEF;
	for (int i=0; i<NullPort->fin.size(); i++)
		NullPort->fin[i]->type = UNDEF;
	states.init = parameters;
}

/* [TODO] save I/O states too */
std::vector<int> Transducer::process(std::vector<Word> stream)
{
	for (int i=0; i<stream.size(); i++)
	{
		{
			shared_ptr<Circuit> c = epsilon_circuit;
			c->reset();
			c->set_stream_in(stream[i].val);
			c->set_state_in(states);
			c->tick();
			states = c->get_state_out();
		}

		{
			auto backup = copy_port(states);
			for (int j=0; j<tag_alphabet_size; j++)
				if (stream[i].tag_bitmap[j])
				{
					shared_ptr<Circuit> c = circuits[j];
					c->reset();
					c->set_stream_in(stream[i].tag_bitmap[j]);
					c->set_state_in(backup);
					c->tick();
					states.merge(c->get_state_out());
				}
		}
	}

	{
		shared_ptr<Circuit> c = epsilon_circuit;
		c->reset();
		c->set_stream_in(stream[i].val);
		c->set_state_in(states);
		c->tick();
		states = c->get_state_out();
	}

	return states.fin;
}

std::vector<int> Transducer::get_signature()
{
	return std::vector<int>(1,0);
}

shared_ptr<Circuit> Transducer::get_default_circuit()
{
	shared_ptr<Circuit> c = new Circuit();
	for (int i=0; i<param_number; i++)
	{
		shared_ptr<Gate> gii = new Gate(0, new ConstOp);
		shared_ptr<Gate> gif = new Gate(0, new ConstOp);
		shared_ptr<Gate> goi = new Gate(0, new CopyOp);
		shared_ptr<Gate> gof = new Gate(0, new CopyOp);
		goi->wire_in(gii);
		gii->wire_out(goi);
		gof->wire_in(gif);
		gif->wire_out(gof);
		c->add_gate(gii, STATE_IN_INIT);
		c->add_gate(gif, STATE_IN_FINAL);
		c->add_gate(goi, STATE_OUT_INIT);
		c->add_gate(gof, STATE_OUT_FINAL);
	}
	return c;
}

}
