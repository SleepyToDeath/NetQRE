#ifndef _DT_TRANSDUCER_H
#define _DT_TRANSDUCER_H

#include "circuit.h"
#include "gate.h"
#include "op.h"

namespace DT
{

	class Word
	{
		int key;
		int val;
	};
	


	class Transducer
	{
		public:
		Transducer(int state_number, int param_number, int final_number, int max_character);
		~Transducer();

		void add_circuit(Circuit* c, int character);
		int combine(Transducer* dt, CombineType t);

		void init(std::vector<int> parameters); /* input the initial values for ALL states */
		std::vector<int> process(std::vector<std::pair<int,int> > stream); /* start/continue to process stream, return result so far */

		std::vector<int> get_signature();

		Circuit* get_default_circuit();

		private:
		int state_number;
		int param_number;
		int final_number;
		int max_character;

		std::vector<int> states;
		std::vector<Circuit*> circuits; /* index = cooresponding character */
	};
}

#endif

