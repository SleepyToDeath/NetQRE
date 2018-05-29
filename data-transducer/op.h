#ifndef _DT_OP_H
#define _DT_OP_H

namespace DT
{

	/* Define values for undefined and conflict. All valid values must be non-negtive */
	enum Constants{
		UNDEF = -1, CONF = -2, MINIMAL_VALID_VALUE = 0;
	};

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

	class UnionOp
	{
		public:
		int operator ()(vector<int> param, int current);
	}

	class BasicBinaryOp
	{
		public:
		int operator ()(vector<int> param, int current);
	}

}

#endif
