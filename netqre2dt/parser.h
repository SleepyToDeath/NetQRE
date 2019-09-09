#ifndef NETQRE2DT_PARSER_H
#define NETQRE2DT_PARSER_H

#include <memory>
#include "syntax.h"
#include "rubify.hpp"

namespace Netqre {

class NetqreParser
{
	public:
	std::shared_ptr<NetqreAST> parse(Rubify::string code);

	private:
	void real_parse(Rubify::string &code, int &cursor, std::shared_ptr<NetqreAST> context);

};

}

#endif
