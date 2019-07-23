#ifndef _DT_GATE_H
#define _DT_GATE_H

#include "op.h"

using std::weak_ptr;

namespace DT
{
	/*
		SEQ: sequential logic, only accept result from previous cycle
		CMB: combinational logic, only accept result from current cycle
	*/
	/* [?] Why did I need this? I can't recall. */
	/*
	enum WireType {
		SEQ, CMB
	};
	
	class Gate;

	class Wire
	{
		public:
		wire(){};
		wire(shared_ptr<Gate> g0, WireType t0):g(g0),t(t0){};
		shared_ptr<Gate> g;
		WireType t;
	};
	*/


	/*
		Every time a gate receives a posedges, 
		it checks if ready_wires = cmb_wires.
		If not, it does nothing.
		If yes, it computes its output,
		and then sends a ready signal to all output cmb wires,
		and then sends a posedge to all output wires.

		Sending a posedge to all gates in any order
		will trigger a clock cycle.
		Computations will be made in topologically sorted order.

		Negedge will reset ready_wires to 0.

		Sending a negedge to all gates means the end of a clock cycle.
	*/

	/* [!] every circuit should be reset every cycle. The only state
			passed to next cycle is the state set. klenee star should
			be expressed within this abstraction. Therefore SEQ is 
			basically useless. But I keep it here just in case. */

	class Gate
	{
		public:
		Gate(shared_ptr<Op> op, std::string name);
		/* only copy op, other fields are reset */
		Gate(shared_ptr<Gate> src);

		void wire_in_seq(shared_ptr<Gate> src); /* add an sequential input wire */
		void wire_out_seq(shared_ptr<Gate> dst); /* add an sequential output wire */
		void wire_in(shared_ptr<Gate> src); /* add an combinational input wire */
		void wire_out(shared_ptr<Gate> dst); /* add an combinational output wire */
		void wire_ready(); /* a input CMB wire is ready to be read for this cycle */
		/* literally set value, should only use for input gates of a circuit */
		void set_value(const unique_ptr<DataValue> &val); 
		void set_op(shared_ptr<Op> op); /* literally set op */
		void posedge(); /* see above */
		void negedge(); /* see above */
		unique_ptr<DataValue> output(); /* get the output value */
		unique_ptr<DataValue> output_seq(); /* get the output value of last cycle*/
		void reset(); /* set val to init, set ready_wires to 0 */

		std::string name;
		int id;

		private:
		shared_ptr<Op> op;
		vector<weak_ptr<Gate> > in;
		vector<weak_ptr<Gate> > in_seq;
		vector<weak_ptr<Gate> > out;
		unique_ptr<DataValue> val, val_old, val_init;
		int cmb_wires;
		int ready_wires;
	};

}

#endif
