#ifndef _DT_CIRCUIT_H
#define _DT_CIRCUIT_H

namespace DT
{
	enum GateType {
		GATE=0, STREAM_IN, STATE_IN, STATE_OUT, NUM_GATE_TYPE
	}

	enum CombineType {
		UNION, PARALLEL, STAR, CONCATENATION
	};

	class Circuit
	{

		public:
		Circuit();

		int add_gate(Gate* g, GateType t); /* return the gate's id */
		Gate* get_gate(int id);
		void combine(Circuit* c);

		void reset();
		void set_state_in(vector<int> states);
		void set_stream_in(int val); /* will set this value to all stream in gates */
		void tick();
		vector<int> get_state_out();

		int combine_char(Circuit* c, CombineType T);
		int combine_epsilon(Circuit* c, CombineType T);

		private:
		vector<Gate*> statei;
		vector<Gate*> stateo;
		vector<Gate*> streami;
		vector<Gate*> gate;

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
