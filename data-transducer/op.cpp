#include "op.h"

namespace DT
{

int ConstOp::operator()(std::vector<int> param, int current)
{
	return current;
}

int CopyOp::operator()(std::vector<int> param, int current)
{
	return param[0];
}

}
