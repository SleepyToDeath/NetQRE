#include "parser.h"
#include <iostream>

using std::cerr;
using std::endl;
using std::unique_ptr;
using std::shared_ptr;
using std::static_pointer_cast;
using Rubify::vector;
using Rubify::string;

namespace Netqre {

std::shared_ptr<NetqreAST> NetqreParser::parse(string code) {
	int cursor = 0;
	auto program = shared_ptr<NetqreAST>(new NetqreAST());
	program->type = NetqreExpType::PROGRAM;
	real_parse(code, cursor, program);
	return program;
}

/* [TODO] This is a naive parser. Switch to lex & bison if possible */
void NetqreParser::real_parse(string &code, int &cursor, shared_ptr<NetqreAST> context) {

#ifdef DT_DEBUG
	cerr<< (int)context->type <<endl;
	cerr<< code.substr(cursor, code.length()-cursor) <<endl;
#endif

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

	auto parse_num = [&]() -> StreamFieldType
	{
		int old_cursor = cursor;
		while (cursor < code.length() && code[cursor]>='0' && code[cursor]<='9')
			cursor++;
		return stoll(code.substr(old_cursor, cursor-old_cursor));
	};
			
	auto parse_binary_num = [&]() -> StreamFieldType
	{
		int old_cursor = cursor;
		while (cursor < code.length() && code[cursor]>='0' && code[cursor]<='1')
			cursor++;
		return stoll(code.substr(old_cursor, cursor-old_cursor), nullptr, 2);
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

			/* [TODO] parse '>', '<', '=' and prefix */
			context->pred_type = PredOpType::NONE;
			skip_head();
			if (code[cursor] == '_')
				parse_it(NetqreExpType::UNKNOWN);
			else
			{
				parse_it(NetqreExpType::FEATURE_NI);
				skip_space();
				switch (code[cursor])
				{
					case '=':
					context->pred_type = PredOpType::EQUAL;
					cursor += 2;
					break;

					case '-':
					context->pred_type = PredOpType::IN;
					cursor += 2;
					break;

					case '>':
					context->pred_type = PredOpType::BIGGER;
					cursor += 2;
					break;

					case '<':
					context->pred_type = PredOpType::SMALLER;
					cursor += 2;
					break;
				}
				parse_it(NetqreExpType::VALUE);
			}
			skip_tail();

			break;

		case NetqreExpType::FEATURE_NI:

			context->value = parse_num();

			break;
		
		case NetqreExpType::VALUE:
			
			context->value = parse_binary_num();

			break;

		case NetqreExpType::QRE_NS:
			switch(code[cursor])
			{
				case '+':
					context->arith_type = ArithOpType::ADD;
					parse_ns();
					break;
				case '-':
					context->arith_type = ArithOpType::SUB;
					parse_ns();
					break;
				case '*':
					context->arith_type = ArithOpType::MUL;
					parse_ns();
					break;
				case '%':
					context->arith_type = ArithOpType::DIV;
					parse_ns();
					break;
				default:
					if (code[cursor] == '/' || (code[cursor] >= 'a' && code[cursor] <= 'z'))
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
						context->agg_type = AggOpType::NONE;
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
					
					default:
					throw string("Syntax error!\n");
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

#ifdef DT_DEBUG
			cerr<<"QRE type "<< (int)context->reg_type<<endl;;
#endif

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
			if(!(code[cursor] == '/' || code[cursor] == ')'))
			{
				switch (code[cursor])
				{
					case '|':
					case '&':
					case '[':
					parse_it(NetqreExpType::PREDICATE_SET);
					break;

					case '*':
					parse_it(NetqreExpType::RE_STAR);
					break;

					case '_':
					parse_it(NetqreExpType::WILDCARD);
					break;

					default:
					throw string("Syntax error!\n");
				}
				skip_space();
			}

			if(!(code[cursor] == '/' || code[cursor] == ')'))
			{
				context->reg_type = RegularOpType::CONCAT;
				parse_it(NetqreExpType::RE);
			}
			else
			{
				context->reg_type = RegularOpType::NONE;
			}

			break;

		case NetqreExpType::RE_STAR:
			context->reg_type = RegularOpType::STAR;
			skip_name();
			parse_it(NetqreExpType::RE);
			skip_tail();
			break;
		

		case NetqreExpType::WILDCARD:
		case NetqreExpType::UNKNOWN:
			context->bool_type = BoolOpType::NONE;
			skip_tail();
			break;
	}

}

}
