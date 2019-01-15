#include "op.hpp"

namespace Netqre {

const bool AndOp::unknown_table[4][4] = {
		{true,	true,	false,	true },
		{true,	true,	false,	true },
		{false,	false,	false,	false },
		{true,	true,	false,	false }
	};
const bool AndOp::truth_table[4][4] = {
		{true,	true,	false,	true },
		{true,	true,	false,	true },
		{false,	false,	false,	false },
		{true,	true,	false,	true }
	};
const bool OrOp::unknown_table[4][4] = {
		{true,	true,	true,	false },
		{true,	true,	true,	false },
		{true,	true,	false,	false },
		{false,	false,	false,	false }
	};
const bool OrOp::truth_table[4][4] = {
		{true,	true,	true,	true },
		{true,	true,	true,	true },
		{true,	true,	false,	true },
		{true,	true,	true,	true }
	};

}

std::unique_ptr<DT::DataValueFactory> DT::DataValue::factory = std::unique_ptr<Netqre::DataValueFactory>(new Netqre::DataValueFactory());
