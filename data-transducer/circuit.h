#ifndef _DT_CIRCUIT_H
#define _DT_CIRCUIT_H

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
		std::vector<int> init;
		std::vector<int> media;
		std::vector<int> fin;
	};

	const std::vector<int> NullPort = std::vector(0);

	class Circuit
	{

		public:
		Circuit();

		int add_gate(Gate* g, GateType t); /* return the gate's id */
		Gate* get_gate(int id);
		void combine(Circuit* c);

		void reset();
		void set_state_in(Port states);
		void set_stream_in(int val); /* will set this value to all stream in gates */
		void tick();
		Port get_state_out();

		int combine_char(Circuit* c, CombineType t);
		int combine_epsilon(Circuit* c, CombineType t);

		void destroy(); /* free all gates */

		private:
		vector<Gate*> statei;
		vector<Gate*> stateii;
		vector<Gate*> stateif;

		vector<Gate*> stateo;
		vector<Gate*> stateoi;
		vector<Gate*> stateof;

		vector<Gate*> streami;

		vector<Gate*> gate;

		vector<Gate*> persistent; /* value won't be cleared by reset */

		int combine_char_union(Circuit* c);
		int combine_char_parallel(Circuit* c);
		int combine_char_concatenation(Circuit* c);
		int combine_char_star();
		int combine_epsilon_union(Circuit* c);
		int combine_epsilon_parallel(Circuit* c);
		int combine_epsilon_concatenation(Circuit* c);
		int combine_epsilon_star();
	};
}

#endif
