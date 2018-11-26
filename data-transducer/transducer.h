#ifndef _DT_TRANSDUCER_H
#define _DT_TRANSDUCER_H

#include "circuit.h"
#include "gate.h"
#include "op.h"

namespace DT
{

	template<class CmpTag>
	class Transducer
	{
		public:
		/* assume in and out have same width */
		Transducer(int param_number);
		~Transducer();

		void add_circuit(std::shared_ptr<Circuit> c, share_ptr<TagValue> tag);
		void add_epsilon_circuit(std::shared_ptr<Circuit> c);
		void combine(std::shared_ptr<Transducer> dt, CombineType t);

		/* 	Input the initial values for init states. 
			other states are assumed to be 0(or undef?) */
		void init(std::vector< unique_ptr<DataValue> > parameters); 

		/* start/continue to process stream, return result so far */
		std::vector<int> process(std::vector<Word> stream ); 

		std::vector<int> get_signature();

		shared_ptr<Circuit> get_default_circuit();

		private:
		int state_number;
		int param_number;

		Port states;
		std::shared_ptr<Circuit> epsilon_circuit;
		std::map<std::shared_ptr<TagValue>, std::shared_ptr<Circuit>, CmpTag> circuits; 
	};
}

#endif

