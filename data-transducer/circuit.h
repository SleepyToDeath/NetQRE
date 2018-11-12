#ifndef _DT_CIRCUIT_H
#define _DT_CIRCUIT_H

#include "gate.h"

namespace DT
{
	enum GateType {
		GATE=0, PERSISTENT, STREAM_IN, STATE_IN, STATE_IN_INIT, STATE_IN_FINAL, STATE_OUT, STATE_OUT_INIT, STATE_OUT_FINAL, NUM_GATE_TYPE
	}

	enum CombineType {
		UNION, PARALLEL, STAR, CONCATENATION
	};

	class Port
	{
		public:
		std::vector<unique_ptr<DataValue> > init;
		std::vector<unique_ptr<DataValue> > media;
		std::vector<unique_ptr<DataValue> > fin;
	};

	const std::vector<unique_ptr<DataValue> > NullPort = std::vector(0);

	class Circuit
	{

		public:
		Circuit();

		int add_gate(std::shared_ptr<Gate> g, GateType t); /* return the gate's id */
		shared_ptr<Gate> get_gate(int id);
		void combine(shared_ptr<Circuit> c);

		/* set_state_in -> set_stream_in -> tick -> get_state_out -> reset */
		void reset();
		void set_state_in(Port states);
		void set_stream_in(unique_ptr<DataValue> val); /* will set this value to all stream in gates */
		void tick();
		Port get_state_out();

		unique_ptr<DataValue> combine_char(shared_ptr<Circuit> c, CombineType t);
		unique_ptr<DataValue> combine_epsilon(shared_ptr<Circuit> c, CombineType t);

		private:
		vector< std::shared_ptr<Gate> > statei; /* main state */
		vector< std::shared_ptr<Gate> > stateii; /* input state */
		vector< std::shared_ptr<Gate> > stateif; /* output state */

		vector< std::shared_ptr<Gate> > stateo; /* main state */
		vector< std::shared_ptr<Gate> > stateoi; /* input state */
		vector< std::shared_ptr<Gate> > stateof; /* output state */

		/* 	value part of a data word
			Can be merged into one gate
			But keep copies for convenience of merging circuits 
			(no need to reconnect wires)
			They are always set to the same value(by set_stream_in) */
		vector< std::shared_ptr<Gate> > streami;

		vector< std::shared_ptr<Gate> > gates; /* [?] what is this? */

//		vector< std::shared_ptr<Gate> > persistent; /* value won't be cleared by reset */ /* [?] don't exist */

		void combine_char_union(std::shared_ptr<Circuit> c);
		void combine_char_parallel(std::shared_ptr<Circuit> c);
		void combine_char_concatenation(std::shared_ptr<Circuit> c);
		void combine_char_star();
		void combine_epsilon_union(std::shared_ptr<Circuit> c);
		void combine_epsilon_parallel(std::shared_ptr<Circuit> c);
		void combine_epsilon_concatenation(std::shared_ptr<Circuit> c);
		void combine_epsilon_star();
	};
}

#endif
