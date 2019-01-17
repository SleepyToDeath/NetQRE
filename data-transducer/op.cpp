#include "op.h"
#include <iostream>
using std::cout;

namespace DT
{

/*
DataValue::DataValue(const unique_ptr<DataValue> &src)
{
	type = src->type;
}

DataValue::DataValue(DataType t)
{
	type = t;
}

DataValue::DataValue()
{
	type = UNDEF;
}
*/
DataValueFactory::~DataValueFactory() {}

std::string DataValue::to_string() { return std::string("[]"); }

DataValue::~DataValue() {}

unique_ptr<DataValue> ConstOp::operator ()(
	const vector< unique_ptr<DataValue> > &param, 
	const unique_ptr<DataValue> &current)
{
	return copy_data(current);
}

unique_ptr<DataValue> CopyOp::operator ()(
	const vector< unique_ptr<DataValue> > &param, 
	const unique_ptr<DataValue> &current)
{
	if (param.size() > 0)
		return copy_data(param[0]);
	else
		return copy_data(current);
}

unique_ptr<DataValue> UnionOp::operator ()(
	const vector< unique_ptr<DataValue> > &param, 
	const unique_ptr<DataValue> &current)
{
	if (param[0]->type == UNDEF)
		return copy_data(param[1]);
	if (param[1]->type == UNDEF)
		return copy_data(param[0]);
	return DataValue::factory->get_instance(CONF);
}

Word::Word() {
}

Word::Word(const Word &src) {
	tag_bitmap.clear();
	for (int i=0; i<src.tag_bitmap.size(); i++)
		tag_bitmap.push_back(copy_data(src.tag_bitmap[i]));
//	tag_bitmap = move(src.tag_bitmap);
}

Word& Word::operator=(const Word &src) {
	if (this != &src)
	{
		for (int i=0; i<src.tag_bitmap.size(); i++)
			tag_bitmap.push_back(copy_data(src.tag_bitmap[i]));
	}
	return *this;
}

}
