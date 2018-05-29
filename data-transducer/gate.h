#ifndef _DT_GATE_H
#define _DT_GATE_H

namespace DT
{
	/*
		SEQ: sequential logic, only accept result from previous cycle
		CMB: combinational logic, only accept result from current cycle
	*/
	enum WireType {
		SEQ, CMB
	};
	
	class Gate;

	class Wire
	{
		public:
		wire(){};
		wire(Gate* g0, WireType t0):g(g0),t(t0){};
		Gate* g;
		WireType t;
	};

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

	class Gate
	{
		public:
		Gate(int init, Op* op);
		~Gate();

		void wire_in(Wire w); /* add an input wire */
		void wire_out(Wire w); /* add an output wire */
		void wire_ready(); /* a CMB wire is ready to be read for this cycle */
		void set_value(int val); /* literally set value, should only use for input gates of a circuit */
		void set_op(Op* op); /* literally set op */
		void posedge(); /* see above */
		void negedge(); /* see above */
		int output(WireType t); /* get the output value based on wire type */
		void reset(); /* set val to init, set ready_wires to 0 */

		private:
		Op* op;
		std::vector<Wire> in;
		std::vector<Wire> out;
		int val, val_old, val_init;
		int cmb_wires;
		int ready_wires;
	};

}

#endif
