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

int UnionOp::operator()(std::vector<int> param, int current)
{
	if (param[0] == UNDEF)
		return param[1];
	if (param[1] == UNDEF)
		return param[0];
	return CONF;
}

int BasicBinaryOp::operator()(std::vector<int> param, int current)
{
	if (param.size() != 2)
		return CONF;
	if (param[0] == UNDEF || param[1] == UNDEF)
		return UNDEF;
	if (param[0] >= 0 && param[1] >= 0)
		return 0;
	return CONF;
}

}
