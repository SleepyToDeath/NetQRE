#include "parser.h"

using namespace std;

NetqreParser::NetqreParser() {
	parsers[""] = [](string &code, int &cursor) {
	}
}

std::shared_ptr<NetqreAST> NetqreParser::parse(std::string code) {
	int cursor = 0;
	auto program = shared_ptr<NetqreAST>(new NetqreAST());
	program->type = PROGRAM;
	return real_parse(code, cursor, PROGRAM);
}


void NetqreParse::real_parse(std::string &code, int &cursor, shared_ptr<NetqreAST> context) {

	auto skip_name = [&]()
	{
		while (code[cursor] != '(')
			cursor++;
		cursor++;
	}

	auto skip_space = [&]()
	{
		while (code[cursor] == ' ')
			cursor++;
	}

	auto skip_tail = [&]()
	{
		skip_space();
		cursor++;
	}

	auto parse_it = [&, this](NetqreExpType type)
	{
		auto ast = shared_ptr<NetqreAST>(new NetqreAST());
		ast->type = type;
		real_parse(ast);
		context->subtree.push_back(ast);
	}

	auto parse_num = [&]() -> int
	{
		/* [TODO] */
	}

	skip_space();

	switch(context->type)
	{
		case PROGRAM:

			parse_it(FILTER);
			parse_it(QRE);
			parse_it(THRESHOLD);
			break;

		case FILTER:

			skip_name();
			parse_it(PREDICATE_SET);
			skip_tail();
			break

		case QRE:

			skip_name();
			parse_it(QRE_NS);
			skip_tail();
			break;

		case THRESHOLD:

			skip_name();
			parse_it(CONST);
			skip_tail();
			break;

		case CONST:
			
			context->value = parse_num();
			break;

		case PREDICATE_SET:

			switch(code[cursor])
			{
				case '&':
				context->bool_type = AND;
				skip_name();
				parse_it(PREDICATE_SET)
				skip_tail();
				parse_it(PREDICATE_SET);
				skip_tail();
				break;

				case '|':
				context->bool_type = OR;
				skip_name();
				parse_it(PREDICATE_SET)
				skip_tail();
				parse_it(PREDICATE_SET);
				skip_tail();
				break;

				case '[':
				parse_it(PREDICATE);
				break;
			}

			break;

		case PREDICATE:

			cursor++;
			parse_it(FEATURE_NI);
			skip_tail();
			parse_it(VALUE);
			skip_tail();

			break;

		case FEATURE_NI:

			value = parse_num();

			break;
		
		case VALUE:
			
			value = parse_num();

			break;

		case QRE_NS:
			
			auto parse_ns = [&,this]()
			{
				parse_it(QRE_NS);
				skip_tail();
				parse_it(QRE_NS);
				skip_tail();
			}

			switch(code[cursor])
			{
				case '+':
					context->num_type = ADD;
					parse_ns();
					break;
				case '-':
					context->num_type = SUB;
					parse_ns();
					break;
				case '*':
					context->num_type = MUL;
					parse_ns();
					break;
				case '/':
					context->num_type = DIV;
					parse_ns();
					break;
				default:
					if (code[cursor] >= 'a' && code[cursor] <= 'z')
						parse_it(QRE_VS);
					else
						parse_it(CONST);
			}

		case QRE_VS:

			auto parse_vs = [&,this]() 
			{
				skip_name();
				parse_it(QRE_VS);
				skip_tail();
				parse_it(FEATURE_SET);
				skip_tail();
			}
			
			if (code[cursor] >= 'a' && code[cursor] <= 'z')
			{
				switch (code[cursor])
				{
					case 'm':
						if (code[cursor+1] == 'a')
							context->agg_type = MAX;
						else
							context->agg_type = MIN;
						parse_vs();
						break;

					case 's':
						if (code[cursor+1] == 'u')
						{
							context->agg_type = SUM;
							parse_vs();
						}
						else
						{
							parse_it(QRE_PS);
						}
						break;

					case 'a':
						context->agg_type = AVG;
						parse_vs();
						break;

					case 'i':
						parse_it(QRE_PS);
						break;
				}
			}
			else if (code[cursor] == '/')
			{
				parse_it(QRE_PS);
			}
			else
			{
				parse_it(CONST);
			}

			break;

		case FEATURE_SET:
			
			parse_it(FEATURE_I);
			skip_space();
			while(code[cursor] == ',')
			{
				cursor++;
				parse_it(FEATURE_I);
				skip_space();
			}
			break;

		case FEATURE_I:
			
			context->value = parse_num();
			break;

		case QRE_PS:

			if (code[cursor] >= 'a' && code[cursor] <= 'z')
			{
				switch(code[cursor])
				{
					case 's':
					context->reg_type = CONCAT;
					skip_name();
					parse_it(QRE_PS);
					skip_tail();
					parse_it(QRE_PS);
					skip_tail();
					parse_it(AGG_OP);
					skip_tail();
					break;

					case 'i':
					context->reg_type = STAR;
					skip_name();
					parse_it(QRE_PS);
					skip_tail();
					parse_it(AGG_OP);
					skip_tail();
					break;
				}
			}
			else if (code[cursor] == '/')
			{
				parse_it(QRE_COND);
			}
			else
			{
				parse_it(CONST);
			}

			break;

		case AGG_OP:
			switch (code[cursor])
			{
				case 'm':
					if (code[cursor+1] == 'a')
						context->agg_type = MAX;
					else
						context->agg_type = MIN;
					break;

				case 's':
					context->agg_type = SUM;
					break;

				case 'a':
					context->agg_type = AVG;
					break;
			}
			break;

		case QRE_COND:
			if (code[cursor] == '/')
			{
				cursor++;
				parse_it(RE);
				skip_tail();
				skip_tail();
				skip_tail();
			}
			else
			{
				parse_it(CONST);
			}
			break;

		case RE:
			while(!(code[cursor] == '/' || code[cursor] == ')'));
			{
				switch (code[cursor])
				{
					case '|':
					case '&':
					case '[':
					parse_it(PREDICATE_SET);
					break;

					case '*':
					skip_name();
					parse_it(RE);
					skip_tail();
					break;

					case '_':
					parse_it(WILDCARD);
					break;
				}
				skip_space();
			}
			break;
	}

}
