#ifndef NETQRE2DT_PARSER_H
#define NETQRE2DT_PARSER_H

class NetqreParser
{
	public:
//	NetqreParser();

	std::shared_ptr<NetqreAST> parse(std::string code);

	private:
	void real_parse(std::string &code, int &cursor, std::shared_ptr<NetqreAST> context);

//	std::map< std::string, void (*)(std::string &, int &) > parsers;
//	int max_keyword_length;
};

#endif
