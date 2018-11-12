#include "op.h"

namespace DT
{

unique_ptr<DataValue> ConstOp::operator ()(
	const vector< unique_ptr<DataValue> > &param, 
	const unique_ptr<DataValue> &current)
{
	return current;
}

unique_ptr<DataValue> CopyOp::operator ()(
	const vector< unique_ptr<DataValue> > &param, 
	const unique_ptr<DataValue> &current)
{
	return param[0];
}

unique_ptr<DataValue> UnionOp::operator ()(
	const vector< unique_ptr<DataValue> > &param, 
	const unique_ptr<DataValue> &current)
{
	if (param[0] == UNDEF)
		return param[1];
	if (param[1] == UNDEF)
		return param[0];
	return CONF;
}

}
