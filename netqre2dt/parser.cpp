#include "parser.h"
#include <iostream>

using std::cerr;
using std::endl;
using std::unique_ptr;
using std::shared_ptr;
using std::static_pointer_cast;
using std::make_shared;
using std::tuple;
using Rubify::vector;
using Rubify::string;

namespace Netqre {

std::shared_ptr<NetqreAST> NetqreParser::parse(string code) {
	return structured_parse(code);
	/*
	int cursor = 0;
	auto program = shared_ptr<NetqreAST>(new NetqreAST());
	program->type = NetqreExpType::PROGRAM;
	real_parse(code, cursor, program);
	return program;
	*/
}


std::shared_ptr<NetqreAST> NetqreParser::structured_parse(string code) {


	auto parse_node = [&](int cursor) -> tuple<shared_ptr<NetqreAST>, int>
	{
		auto node = make_shared<NetqreAST>();
		auto tokens = Rubify::string(code.substr(cursor)).split(MarshallDelimiter);
		string node_name = tokens[0];

		auto cur = make_shared<NetqreAST>();

/*
		switch (node_name)
		{
//	PROGRAM, 		//0
			case "program":
			cur->type = NetqreExpType::PROGRAM;
			break;
//	FILTER, 		//1
			case "filter":
			cur->type = NetqreExpType::FILTER;
			break;
//	PREDICATE_SET, 	//2
			case "predicate_set":
			cur->type = NetqreExpType::PREDICATE_SET;
			break;
//	PREDICATE, 		//3
			case "predicate_entry":
			cur->type = NetqreExpType::PREDICATE;
			break;
//	FEATURE_NI, 	//4
			case "feature_ni":
			cur->type = NetqreExpType::FEATURE_NI;
			break;
//	VALUE, 			//5
			case "value":
			cur->type = NetqreExpType::VALUE;
			break;
//	QRE,			//6
			case "qre":
			cur->type = NetqreExpType::QRE;
			break;
//	QRE_NS, 		//7
			case "qre_ns":
			cur->type = NetqreExpType::QRE_NS;
			break;
//	NUM_OP, 		//8
			case "num_op":
			cur->type = NetqreExpType::NUM_OP;
			break;
//	QRE_VS, 		//9
			case "qre_vs":
			cur->type = NetqreExpType::QRE_VS;
			break;
//	AGG_OP, 		//10
			case "agg_op":
			cur->type = NetqreExpType::AGG_OP;
			break;
//	FEATURE_SET,	//11
			case "feature_set":
			cur->type = NetqreExpType::FEATURE_SET;
			break;
//	FEATURE_I, 		//12
			case "filter":
			cur->type = NetqreExpType::FEATURE_I;
			break;
//	QRE_PS,			//13
			case "qre_ps":
			cur->type = NetqreExpType::QRE_PS;
			break;
//	QRE_COND,		//14
			case "qre_cond":
			cur->type = NetqreExpType::QRE_COND;
			break;
//	RE,				//15
			case "re":
			cur->type = NetqreExpType::RE;
			break;
//	RE_STAR,		//16
			case "re_star":
			cur->type = NetqreExpType::RE_STAR;
			break;
//	OUTPUT,			//17
			case "output":
			cur->type = NetqreExpType::OUTPUT;
			break;
//	WILDCARD,		//18
			case "wildcard":
			cur->type = NetqreExpType::WILDCARD;
			break;
//	CONST,			//19
			case "const":
			cur->type = NetqreExpType::CONST;
			break;
//	THRESHOLD,		//20
			case "threshold":
			cur->type = NetqreExpType::THRESHOLD;
			break;
//	UNKNOWN			//21
			case "unknown":
			cur->type = NetqreExpType::UNKNOWN;
			break;
		}
		*/

	};
	auto [ast, cursor] = parse_node(1);
	return ast;
}





























/* ------------------------- Obsolete ----------------------------- */


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
