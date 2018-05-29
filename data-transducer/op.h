#ifndef _DT_OP_H
#define _DT_OP_H

namespace DT
{

	/*
		All true operators must be subclass of op
		and override () operator.
	*/
	class Op
	{
		public:
		virtual int operator ()(vector<int> l, int current)=0;
	};

	class ConstOp
	{
		public:
		int operator ()(vector<int> param, int current);
	};

	class CopyOp
	{
		public:
		int operator ()(vector<int> param, int current);
	};


}

#endif
