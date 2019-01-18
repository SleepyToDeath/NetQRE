#include "transducer.h"
#include <iostream>

using std::string;
using std::shared_ptr;
using std::unique_ptr;
using std::cout;
using std::endl;

namespace DT
{

Transducer::Transducer(int tag_alphabet_size, std::shared_ptr<MergeParallelOp> state_merger)
{
	cout<<"Alphabet size: "<<tag_alphabet_size<<endl;
	this->tag_alphabet_size = tag_alphabet_size;
	this->state_merger = state_merger;
	circuits = vector< shared_ptr<Circuit> >(tag_alphabet_size, nullptr); //[!] assuming circuits is a vector
	epsilon_circuit = nullptr;
}

void Transducer::add_circuit(shared_ptr<Circuit> c, TagType tag)
{
	cout<<"Adding circuit #"<<tag<<endl;
	circuits[tag] = c;
}

void Transducer::add_epsilon_circuit(std::shared_ptr<Circuit> c)
{
	epsilon_circuit = c;
}

void Transducer::combine(shared_ptr<Transducer> dt, CombineType t, std::shared_ptr<PipelineOp> init_op, std::shared_ptr<PipelineOp> commit_op)
{
	if (dt==nullptr)
	{
		if (t!=CombineType::STAR && t!=CombineType::CONDITIONAL)
			throw string("Combining with empty!\n");
		epsilon_circuit->combine_epsilon(nullptr, t, state_merger, init_op, commit_op);
		for (int i=0; i<circuits.size(); i++)
			if (circuits[i] != nullptr)
			{
				circuits[i]->combine_char(nullptr,t);
				if (epsilon_circuit->size() != circuits[i]->size())
					throw string("Incorrect combine!\n");
			}
	}
	else
	{
		auto plain1 = epsilon_circuit->get_plain_circuit();
		auto plain2 = dt->epsilon_circuit->get_plain_circuit();
		cout<<plain1->size()<<" : "<<plain2->size()<<endl;
		epsilon_circuit->combine_epsilon(dt->epsilon_circuit,t, state_merger, init_op, commit_op);
		for (int i=0; i<circuits.size(); i++)
		{
			if (circuits[i]!=nullptr || dt->circuits[i]!=nullptr)
			{
				if (circuits[i]==nullptr)
					circuits[i] = plain1->get_plain_circuit();
				if (dt->circuits[i]==nullptr)
					dt->circuits[i] = plain2->get_plain_circuit();
				cout<<circuits[i]->size()<<" : "<<dt->circuits[i]->size()<<endl;
				circuits[i]->combine_char(dt->circuits[i],t);
				cout<<epsilon_circuit->size()<<" : "<<circuits[i]->size()<<endl<<endl;
				if (epsilon_circuit->size() != circuits[i]->size())
				{
					throw string("Incorrect combine!\n");
				}
			}
		}
	}
}

void Transducer::reset(const std::vector<unique_ptr<DataValue> > &parameters)
{
	NullPort = epsilon_circuit->get_state_out();

	if (parameters.size() != NullPort->init.size())
		throw string("Parameter size mismatch!\n");

	for (int i=0; i<NullPort->init.size(); i++)
		NullPort->init[i]->type = UNDEF;
	for (int i=0; i<NullPort->media.size(); i++)
		NullPort->media[i]->type = UNDEF;
	for (int i=0; i<NullPort->fin.size(); i++)
		NullPort->fin[i]->type = UNDEF;

	states = copy_port(NullPort);
	for (int i=0; i<parameters.size(); i++)
		states->init[i] = copy_data(parameters[i]);
}

std::vector< unique_ptr<DataValue> > Transducer::process(std::vector<Word> &stream)
{
	int count_max = 3;
	for (int i=0; i<stream.size(); i++)
	{
		cout<<endl<<"Word # epsilon "<<i<<endl;
		int count;
		if (i==0)
			count = 1;
		else
			count = count_max;
		for (int j=0; j<count; j++)
		{
			shared_ptr<Circuit> c = epsilon_circuit;
			c->reset();
			c->set_stream_in(DataValue::factory->get_instance(UNDEF));
			c->set_state_in(states);
			c->tick();
			states = c->get_state_out();
		}

		cout<<endl<<"Word # char "<<i<<endl;
		{
//			auto backup = copy_port(states);
//			states = copy_port(NullPort);
			for (int j=0; j<tag_alphabet_size; j++)
				if (stream[i].tag_bitmap[j])
				{
					shared_ptr<Circuit> c = circuits[j];
					if (c == nullptr)
						throw string("Impossible!!!!!!!\n");
					c->reset();
					c->set_stream_in(stream[i].tag_bitmap[j]);
					c->set_state_in(states);
					c->tick();
//					states->merge(c->get_state_out(), state_merger);
					states = c->get_state_out();
				}
		}
	}

	for (int j=0; j<count_max; j++)
	{
		cout<<endl<<"Final epsilon "<<endl;
		shared_ptr<Circuit> c = epsilon_circuit;
		c->reset();
		c->set_stream_in(DataValue::factory->get_instance(UNDEF));
		c->set_state_in(states);
		c->tick();
		states = c->get_state_out();
	}

	vector<unique_ptr<DataValue> > ans;
	for (int i=0; i<states->fin.size(); i++)
	{
		ans.push_back(copy_data(states->fin[i]));
	}
	return ans;
}

std::vector<int> Transducer::get_signature()
{
	return std::vector<int>(1,0);
}

/*
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
*/

}
