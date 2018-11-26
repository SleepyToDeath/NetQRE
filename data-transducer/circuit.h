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
		Port();
		Port(const std::unique_ptr<Port> &src);
		std::vector<std::unique_ptr<DataValue> > init;
		std::vector<std::unique_ptr<DataValue> > media;
		std::vector<std::unique_ptr<DataValue> > fin;
	};

	class Circuit
	{

		public:
		Circuit();

		void add_gate(std::shared_ptr<Gate> g, GateType t);
		void combine(shared_ptr<Circuit> c);

		/* set_state_in -> set_stream_in -> tick -> get_state_out -> reset */
		void reset();
		void set_state_in(const unique_ptr<Port> &states);
		/* will propagate this value to all stream in gates */
		void set_stream_in(const unique_ptr<DataValue> &val); 
		void tick();
		std::unique_ptr<Port> get_state_out();

		/* the original c will be destroyed; this circuit will become the combined one */
		std::unique_ptr<DataValue> combine_char(shared_ptr<Circuit> c, CombineType t);
		std::unique_ptr<DataValue> combine_epsilon(shared_ptr<Circuit> c, CombineType t);

		private:
		/* [!] gates of ii, io, oi, of must be of the same number and be aligned */
		vector< std::shared_ptr<Gate> > statei; /* input main state */
		vector< std::shared_ptr<Gate> > stateii; /* input init state */
		vector< std::shared_ptr<Gate> > stateif; /* input fin state */

		vector< std::shared_ptr<Gate> > stateo; /* output main state */
		vector< std::shared_ptr<Gate> > stateoi; /* output init state */
		vector< std::shared_ptr<Gate> > stateof; /* output fin state */

		/* 	value part of a data word
			Can be merged into one gate
			But keep copies for convenience of merging circuits 
			(no need to reconnect wires)
			They are always set to the same value(by set_stream_in) */
		vector< std::shared_ptr<Gate> > streami;

//		vector< std::shared_ptr<Gate> > gates; /* internal gates */
		vector< std::shared_ptr<Gate> > gates; /* all gates */

		void combine_basic(std::shared_ptr<Circuit> c);

		/* [!]  Parallel in this version is actually not
				the intended parallel. It's a generalized
				version of union.
				The original parallel operation will
				make the width of fin state greater
				than in, which makes the resulting circuit
				incompatible with kleene star */
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
