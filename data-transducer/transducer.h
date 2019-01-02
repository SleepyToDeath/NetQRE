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

		/* ================= building ================= */

			/* assume in and out have same width */
			Transducer(int param_number, int tag_alphabet_size, std::shared_ptr<MergeParallelOp> state_merger);
			~Transducer();

			void add_circuit(std::shared_ptr<Circuit> c, TagType tag);
			void add_epsilon_circuit(std::shared_ptr<Circuit> c);
			void combine(std::shared_ptr<Transducer> dt, CombineType t, std::shared_ptr<PipelineOp> init_op, std::shared_ptr<MergeParallelOp> commit_op);

			std::vector<int> get_signature();
			shared_ptr<Circuit> get_default_circuit();

		/* ============================================= */



		/* ================ Execution ================== */

			/* 	Input the initial values for init states. 
				other states are assumed to be 0(or undef?) */
			void reset(std::vector< unique_ptr<DataValue> > parameters); 

			/* start/continue to process stream, return result so far */
			std::vector< unique_ptr<DataValue> > process(std::vector<Word> &stream ); 

		/* ============================================= */

		private:
//		int param_number; // useless for now
		int tag_alphabet_size;
		std::shared_ptr<MergeParallelOp> state_merger;

		std::unique_ptr<Port> states;
		std::shared_ptr<Circuit> epsilon_circuit;
//		std::map< TagType, std::shared_ptr<Circuit> > circuits; // use this instead if TagType is not int
		std::vector< std::shared_ptr<Circuit> > circuits; 
		std::unique_ptr<Port> NullPort;

	};
}

#endif

