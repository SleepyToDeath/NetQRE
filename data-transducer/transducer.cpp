#include "transducer.h"

namespace DT
{

Transducer::Transducer(int state_number, int param_number, int final_number, int max_character)
{
	this->state_number = state_number;
	this->param_number = param_number;
	this->final_number = final_number;
	this->max_character = max_character;
	states = std::vector<int>(state_number+1);
	circuits = std::vector<int>(max_character);
}

Transducer::~Transducer()
{

}

void Transducer::add_circuit(shared_ptr<Circuit> c, int character)
{
	circuits[character] = c;
}

int Transducer::combine(shared_ptr<Transducer> dt, CombineType t)
{
	circuits[0]->combine_epsilon(dt->circuits[0],t);
	for (int i=1; i<max_character; i++)
		circuits[i]->combine_char(dt->circuits[i],t);
	return 0;
}

int Transducer::init(std::vector<int> parameters)
{
	states.init = parameters;
}

std::vector<int> Transducer::process(std::vector<Word> stream)
{
	for (int i=0; i<stream.size(); i++)
	{
		{
			states.fin = NullPort;
			shared_ptr<Circuit> c = circuits[0];
			c->reset();
			c->set_stream_in(stream[i].val);
			c->set_state_in(states);
			c->tick();
			states = c->get_state_out();
			states.init = NullPort;
		}

		{
			states.fin = NullPort;
			shared_ptr<Circuit> c = circuits[stream[i].key];
			c->reset();
			c->set_stream_in(stream[i].val);
			c->set_state_in(states);
			c->tick();
			states = c->get_state_out();
			states.init = NullPort;
		}
	}

	{
		states.fin = NullPort;
		shared_ptr<Circuit> c = circuits[0];
		c->reset();
		c->set_stream_in(stream[i].val);
		c->set_state_in(states);
		c->tick();
		states = c->get_state_out();
		states.init = NullPort;
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
	for (int i=0; i<state_number; i++)
	{
		shared_ptr<Gate> in = new Gate(0, new ConstOp);
		shared_ptr<Gate> out = new Gate(0, new CopyOp);
		out->wire_in(Wire(in,CMB));
		in->wire_out(Wire(out,CMB));
		c->add_gate(in, STATE_IN);
		c->add_gate(out, STATE_OUT);
	}
	return c;
}

}
