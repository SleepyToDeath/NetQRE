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

void Transducer::add_circuit(Circuit* c, int character)
{
	circuits[character] = c;
}

int Transducer::combine(Transducer* dt, CombineType t)
{
	switch (t)
	{
		case UNION:
		return combine_union(dt);

		case STAR:
		return combine_star(dt);

		case CONCATENATION:
		return combine_concatenation(dt);

		case PARALLEL:
		return combine_parallel(dt);

		default:
		return -1;
	}
}

int Transducer::init(std::vector<int> parameters)
{
	states = parameters;
}

int Transducer::process(std::vector<Word> stream)
{
	for (int i=0; i<stream.size(); i++)
	{
		{
			Circuit* c = circuits[stream[i].key];
			c->reset();
			c->set_stream_in(stream[i].val);
			c->set_state_in(states);
			c->tick();
			states = c->get_state_out();
			for (int j=0; j<param_number; j++)
				states[j] = 0;
		}

		{
			Circuit* c = circuits[0];
			c->set_stream_in(stream[i].val);
			c->reset();
			c->set_state_in(states);
			c->tick();
			states = c->get_state_out();
		}
	}
}

std::vector<int> Transducer::get_signature()
{
	return std::vector<int>(1,0);
}

Circuit* Transducer::get_default_circuit()
{
	Circuit* c = new Circuit();
	for (int i=0; i<state_number; i++)
	{
		Gate* in = new Gate(0, new ConstOp);
		Gate* out = new Gate(0, new CopyOp);
		out->wire_in(Wire(in,CMB));
		in->wire_out(Wire(out,CMB));
		c->add_gate(in, STATE_IN);
		c->add_gate(out, STATE_OUT);
	}
	return c;
}

}
