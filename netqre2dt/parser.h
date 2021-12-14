#ifndef NETQRE2DT_PARSER_H
#define NETQRE2DT_PARSER_H

#include <memory>
#include "syntax.h"
#include "rubify.hpp"
#include "../general-lang/syntax_tree.h"

namespace Netqre {

class NetqreParser
{
	public:
	std::shared_ptr<NetqreAST> parse(Rubify::string code);

	private:
	std::shared_ptr<NetqreAST> structured_parse(Rubify::string code);
	void real_parse(Rubify::string &code, int &cursor, std::shared_ptr<NetqreAST> context);

};


const std::map<std::string, Netqre::NetqreExpType> ExpTypeMap = {
	{"program", NetqreExpType::PROGRAM},
	{"#filter", NetqreExpType::FILTER},
	{"#threshold", NetqreExpType::THRESHOLD},
	{"#qre_top", NetqreExpType::QRE},
	{"#qre_vs", NetqreExpType::QRE_VS},
	{"#qre_ps", NetqreExpType::QRE_PS},
	{"#agg_op", NetqreExpType::AGG_OP},
	{"#feature_set", NetqreExpType::FEATURE_SET},
	{"#re", NetqreExpType::RE},
	{"#predicate_set", NetqreExpType::PREDICATE_SET},
	{"#predicate_entry", NetqreExpType::PREDICATE},
	{"#index", NetqreExpType::VALUE_BIN},
	{"#prefix", NetqreExpType::VALUE_BIN},
	{"#cap_index", NetqreExpType::VALUE_BIN},
	{"#bottom_index", NetqreExpType::VALUE_BIN},
	{"_", NetqreExpType::WILDCARD},
	{"*_", NetqreExpType::RE_STAR}
};

}

#endif
