#ifndef _DT_CIRCUIT_H
#define _DT_CIRCUIT_H

#include<memory>

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

		int add_gate(std::shared_ptr<Gate> g, GateType t); /* return the gate's id */
		shared_ptr<Gate> get_gate(int id);
		void combine(shared_ptr<Circuit> c);

		void reset();
		void set_state_in(Port states);
		void set_stream_in(int val); /* will set this value to all stream in gates */
		void tick();
		Port get_state_out();

		int combine_char(shared_ptr<Circuit> c, CombineType t);
		int combine_epsilon(shared_ptr<Circuit> c, CombineType t);

		void destroy(); /* free all gates */

		private:
		vector< std::shared_ptr<Gate> > statei;
		vector< std::shared_ptr<Gate> > stateii;
		vector< std::shared_ptr<Gate> > stateif;

		vector< std::shared_ptr<Gate> > stateo;
		vector< std::shared_ptr<Gate> > stateoi;
		vector< std::shared_ptr<Gate> > stateof;

		vector< std::shared_ptr<Gate> > streami;

		vector< std::shared_ptr<Gate> > gate;

		vector< std::shared_ptr<Gate> > persistent; /* value won't be cleared by reset */

		int combine_char_union(std::shared_ptr<Circuit> c);
		int combine_char_parallel(std::shared_ptr<Circuit> c);
		int combine_char_concatenation(std::shared_ptr<Circuit> c);
		int combine_char_star();
		int combine_epsilon_union(std::shared_ptr<Circuit> c);
		int combine_epsilon_parallel(std::shared_ptr<Circuit> c);
		int combine_epsilon_concatenation(std::shared_ptr<Circuit> c);
		int combine_epsilon_star();
	};
}

#endif
