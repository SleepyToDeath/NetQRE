#ifndef _DT_TRANSDUCER_H
#define _DT_TRANSDUCER_H

#include "circuit.h"
#include "gate.h"
#include "op.h"

namespace DT
{


	class Transducer
	{
		public:
		Transducer(int state_number, int param_number, int final_number);
		~Transducer();

		void add_circuit(std::shared_ptr<Circuit> c, int character);
		int combine(std::shared_ptr<Transducer> dt, CombineType t);

		void init(std::vector< unique_ptr<DataValue> > parameters); /* input the initial values for init states. other states are assumed to be 0(or undef?) */
		std::vector<int> process(std::vector<std::pair<int,int> > stream); /* start/continue to process stream, return result so far */

		std::vector<int> get_signature();

		shared_ptr<Circuit> get_default_circuit();

		private:
		int state_number;
		int param_number;
		int final_number;

		Port states;
		std::vector< std::shared_ptr<Circuit> > circuits; 
	};
}

#endif

