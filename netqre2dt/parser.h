#ifndef NETQRE2DT_PARSER_H
#define NETQRE2DT_PARSER_H

#include <memory>
#include "syntax.h"

namespace Netqre {

class NetqreParser
{
	public:
	std::shared_ptr<NetqreAST> parse(std::string code);

	private:
	void real_parse(std::string &code, int &cursor, std::shared_ptr<NetqreAST> context);

};

}

#endif
