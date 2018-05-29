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
		Every time a gate receive a posedges, 
		it checks if ready_wires = cmb_wires.
		If not, it does nothing.
		If yes, it computes its output,
		and then sends a ready signal to all output cmb wires,
		and then sends a posedge to all output wires.

		Sending a posedge to all gates in any order
		will trigger a cycle.
		Computations will be made in topological sort order.

		Negedge will reset ready_wires to 0.

		Sending a negedge to all gates means the end of a cycle.
	*/

	class Gate
	{
		public:
		Gate(int init, Op* op);
		~Gate();

		void wire_in(Wire w);
		void wire_out(Wire w);
		void wire_ready();
		void set_value(int val);
		void posedge();
		void negedge();
		int output(WireType t);
		void reset();

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
