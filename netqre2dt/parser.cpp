#include "parser.h"
#include <string>
#include <iostream>

using std::cout;
using std::endl;
using std::unique_ptr;
using std::shared_ptr;
using std::string;
using std::vector;
using std::static_pointer_cast;

namespace Netqre {

std::shared_ptr<NetqreAST> NetqreParser::parse(std::string code) {
	int cursor = 0;
	auto program = shared_ptr<NetqreAST>(new NetqreAST());
	program->type = NetqreExpType::PROGRAM;
	real_parse(code, cursor, program);
	return program;
}


void NetqreParser::real_parse(std::string &code, int &cursor, shared_ptr<NetqreAST> context) {

	cout<< (int)context->type <<endl;
	cout<< code.substr(cursor, code.length()-cursor) <<endl;

	auto skip_name = [&]()
	{
		while (code[cursor] != '(')
			cursor++;
		cursor++;
	};

	auto skip_space = [&]()
	{
		while (cursor < code.length() && code[cursor] == ' ')
			cursor++;
	};

	auto skip_head = [&]()
	{
		cursor++;
		skip_space();
	};

	auto skip_tail = [&]()
	{
		skip_space();
		cursor++;
	};

	auto parse_it = [&, this](NetqreExpType type)
	{
		auto ast = shared_ptr<NetqreAST>(new NetqreAST());
		ast->type = type;
		real_parse(code, cursor, ast);
		context->subtree.push_back(ast);
	};

	auto parse_num = [&]() -> int
	{
		int old_cursor = cursor;
		while (cursor < code.length() && code[cursor]>='0' && code[cursor]<='9')
			cursor++;
		return stoi(code.substr(old_cursor, cursor-old_cursor));
	};
			
	auto parse_ns = [&,this]()
	{
		parse_it(NetqreExpType::QRE_NS);
		skip_tail();
		parse_it(NetqreExpType::QRE_NS);
		skip_tail();
	};

	auto parse_vs = [&,this]() 
	{
		skip_name();
		parse_it(NetqreExpType::QRE_VS);
		skip_tail();
		parse_it(NetqreExpType::FEATURE_SET);
		skip_tail();
	};

	skip_space();

	switch(context->type)
	{
		case NetqreExpType::PROGRAM:

			parse_it(NetqreExpType::FILTER);
			parse_it(NetqreExpType::QRE);
			parse_it(NetqreExpType::THRESHOLD);
			break;

		case NetqreExpType::FILTER:

			skip_name();
			parse_it(NetqreExpType::PREDICATE_SET);
			skip_tail();
			break;

		case NetqreExpType::QRE:

			skip_name();
			parse_it(NetqreExpType::QRE_NS);
			skip_tail();
			break;

		case NetqreExpType::THRESHOLD:

			skip_name();
			parse_it(NetqreExpType::CONST);
			skip_tail();
			break;

		case NetqreExpType::CONST:
			
			context->value = parse_num();
			break;

		case NetqreExpType::PREDICATE_SET:

			switch(code[cursor])
			{
				case '&':
				context->bool_type = BoolOpType::AND;
				skip_name();
				parse_it(NetqreExpType::PREDICATE_SET);
				skip_tail();
				parse_it(NetqreExpType::PREDICATE_SET);
				skip_tail();
				break;

				case '|':
				context->bool_type = BoolOpType::OR;
				skip_name();
				parse_it(NetqreExpType::PREDICATE_SET);
				skip_tail();
				parse_it(NetqreExpType::PREDICATE_SET);
				skip_tail();
				break;

				case '[':
				context->bool_type = BoolOpType::NONE;
				parse_it(NetqreExpType::PREDICATE);
				break;

			}

			break;

		case NetqreExpType::PREDICATE:

			skip_head();
			if (code[cursor] == '_')
				parse_it(NetqreExpType::UNKNOWN);
			else
			{
				parse_it(NetqreExpType::FEATURE_NI);
				skip_tail();
				parse_it(NetqreExpType::VALUE);
			}
			skip_tail();

			break;

		case NetqreExpType::FEATURE_NI:

			context->value = parse_num();

			break;
		
		case NetqreExpType::VALUE:
			
			context->value = parse_num();

			break;

		case NetqreExpType::QRE_NS:
			switch(code[cursor])
			{
				case '+':
					context->num_type = NumOpType::ADD;
					parse_ns();
					break;
				case '-':
					context->num_type = NumOpType::SUB;
					parse_ns();
					break;
				case '*':
					context->num_type = NumOpType::MUL;
					parse_ns();
					break;
				case '/':
					context->num_type = NumOpType::DIV;
					parse_ns();
					break;
				default:
					if (code[cursor] >= 'a' && code[cursor] <= 'z')
						parse_it(NetqreExpType::QRE_VS);
					else
						parse_it(NetqreExpType::CONST);
			}

			break;

		case NetqreExpType::QRE_VS:

		
			if (code[cursor] >= 'a' && code[cursor] <= 'z')
			{
				switch (code[cursor])
				{
					case 'm':
						if (code[cursor+1] == 'a')
							context->agg_type = AggOpType::MAX;
						else
							context->agg_type = AggOpType::MIN;
						parse_vs();
						break;

					case 's':
						if (code[cursor+1] == 'u')
						{
							context->agg_type = AggOpType::SUM;
							parse_vs();
						}
						else
						{
							context->agg_type = AggOpType::NONE;
							parse_it(NetqreExpType::QRE_PS);
						}
						break;

					case 'a':
						context->agg_type = AggOpType::AVG;
						parse_vs();
						break;

					case 'i':
						parse_it(NetqreExpType::QRE_PS);
						break;
				}
			}
			else if (code[cursor] == '/')
			{
				context->agg_type = AggOpType::NONE;
				parse_it(NetqreExpType::QRE_PS);
			}
			else
			{
				context->agg_type = AggOpType::NONE;
				parse_it(NetqreExpType::CONST);
			}

			break;

		case NetqreExpType::FEATURE_SET:
			
			parse_it(NetqreExpType::FEATURE_I);
			skip_space();
			while(code[cursor] == ',')
			{
				skip_head();
				parse_it(NetqreExpType::FEATURE_I);
				skip_space();
			}
			break;

		case NetqreExpType::FEATURE_I:
			
			context->value = parse_num();
			break;

		case NetqreExpType::QRE_PS:

			if (code[cursor] >= 'a' && code[cursor] <= 'z')
			{
				switch(code[cursor])
				{
					case 's':
					context->reg_type = RegularOpType::CONCAT;
					skip_name();
					parse_it(NetqreExpType::QRE_PS);
					skip_tail();
					parse_it(NetqreExpType::QRE_PS);
					skip_tail();
					parse_it(NetqreExpType::AGG_OP);
					skip_tail();
					break;

					case 'i':
					context->reg_type = RegularOpType::STAR;
					skip_name();
					parse_it(NetqreExpType::QRE_PS);
					skip_tail();
					parse_it(NetqreExpType::AGG_OP);
					skip_tail();
					break;
				}
			}
			else if (code[cursor] == '/')
			{
				context->reg_type = RegularOpType::NONE;
				parse_it(NetqreExpType::QRE_COND);
			}
			else
			{
				context->reg_type = RegularOpType::NONE;
				parse_it(NetqreExpType::CONST);
			}

			cout<<"QRE type "<< (int)context->reg_type<<endl;;

			break;

		case NetqreExpType::AGG_OP:
			switch (code[cursor])
			{
				case 'm':
					if (code[cursor+1] == 'a')
						context->agg_type = AggOpType::MAX;
					else
						context->agg_type = AggOpType::MIN;
					break;

				case 's':
					context->agg_type = AggOpType::SUM;
					break;

				case 'a':
					context->agg_type = AggOpType::AVG;
					break;
			}
			while(cursor < code.length() && code[cursor] >= 'a' && code[cursor] <= 'z')
				cursor++;
			break;

		case NetqreExpType::QRE_COND:
			if (code[cursor] == '/')
			{
				cursor++;
				parse_it(NetqreExpType::RE);
				skip_tail();
				skip_tail();
				parse_num();
			}
			else
			{
				parse_it(NetqreExpType::CONST);
			}
			break;

		case NetqreExpType::RE:
			while(!(code[cursor] == '/' || code[cursor] == ')'))
			{
				switch (code[cursor])
				{
					case '|':
					case '&':
					case '[':
					parse_it(NetqreExpType::PREDICATE_SET);
					break;

					case '*':
					skip_name();
					parse_it(NetqreExpType::RE);
					skip_tail();
					break;

					case '_':
					parse_it(NetqreExpType::WILDCARD);
					break;
				}
				skip_space();
			}
			break;

		case NetqreExpType::WILDCARD:
		case NetqreExpType::UNKNOWN:
			skip_tail();
			break;
	}

}

}
