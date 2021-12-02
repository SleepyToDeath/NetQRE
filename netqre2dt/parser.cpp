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


	auto parse_node = [&](int cursor, auto& self) -> tuple<shared_ptr<NetqreAST>, int>
	{
		if (code[cursor] == MarshallRight[0])
			return {nullptr, cursor+1};
		if (code[cursor] == MarshallLeft[0])
			cursor++;
		auto node = make_shared<NetqreAST>();
		auto tokens = Rubify::string(code.substr(cursor)).split(MarshallDelimiter);

		/* initialize this node */
		auto cur = make_shared<NetqreAST>();
		string node_name = tokens[0];
		cur->name = node_name;
		if (ExpTypeMap.contains(node_name))
			cur->type = ExpTypeMap.at(node_name);
		else
			cur->type = NetqreExpType::PENDING_LITERAL;

		std::cerr<<"{ "<<node_name<<" "<<cursor<<" |";

		/* parse subtree */
		vector< std::shared_ptr<NetqreAST> > pending;
		int cursor2 = cursor + node_name.length();
		while (code[cursor2] == MarshallDelimiter[0])
		{
			/* parse next sub-tree */
			auto [sub, cursor3] = self(cursor2 + 1, self);
			cursor2 = cursor3;

			/* check reaching the end */
			if (sub == nullptr)
				break;

			/* add sub-tree to list */
			pending.push_back(sub);
		}


		switch(cur->type)
		{
			case NetqreExpType::PROGRAM:
			case NetqreExpType::FILTER:
			case NetqreExpType::THRESHOLD:
			case NetqreExpType::QRE_NS:
			case NetqreExpType::FEATURE_SET:
			case NetqreExpType::FEATURE_I:
			case NetqreExpType::QRE_COND:
			case NetqreExpType::RE_STAR:
			case NetqreExpType::OUTPUT:
			case NetqreExpType::WILDCARD:
			case NetqreExpType::CONST:
			{
				cur->subtree = pending;
				break;
			}

			case NetqreExpType::RE:
			{
				switch (pending[0]->type)
				{
					case NetqreExpType::PREDICATE_SET:
					case NetqreExpType::UNKNOWN:
					cur->reg_type = RegularOpType::NONE;
					cur->subtree = pending;
					break;

					case NetqreExpType::RE:
					cur->reg_type = RegularOpType::CONCAT;
					cur->subtree = pending;
					break;

					case NetqreExpType::RE_STAR:
					{
						cur->reg_type = RegularOpType::STAR;
						auto ast_wild = make_shared<NetqreAST>(NetqreExpType::UNKNOWN);
						cur->subtree.push_back(ast_wild);
						break;
					}
				}
				break;
			}

			case NetqreExpType::QRE:
			{
				auto dummy_ns = make_shared<NetqreAST>(NetqreExpType::QRE_NS);
				dummy_ns->subtree.push_back(pending[0]);
				cur->subtree.push_back(dummy_ns);
				break;
			}

			case NetqreExpType::QRE_VS:
			{
				if (pending.size() == 1)
				{
					cur->agg_type = AggOpType::NONE;
					cur->subtree = pending;
					break;
				}
				else if (pending[0]->name == "max")
					cur->agg_type = AggOpType::MAX;
				else if (pending[0]->name == "min")
					cur->agg_type = AggOpType::MIN;
				else if (pending[0]->name == "sum")
					cur->agg_type = AggOpType::SUM;
				cur->subtree.push_back(pending[1]);
				cur->subtree.push_back(pending[2]);
				break;
			}

			case NetqreExpType::QRE_PS:
			{
				if (pending[0]->name == "split")
				{
					cur->reg_type = RegularOpType::CONCAT;
					cur->subtree.push_back(pending[1]);
					cur->subtree.push_back(pending[2]);
					cur->subtree.push_back(pending[3]);
				}
				else if (pending[0]->name == "iter")
				{
					cur->reg_type = RegularOpType::STAR;
					cur->subtree.push_back(pending[1]);
					cur->subtree.push_back(pending[2]);
				}
				else
				{
					cur->reg_type = RegularOpType::NONE;
					auto cond = make_shared<NetqreAST>(NetqreExpType::QRE_COND);
					cur->subtree.push_back(cond);
					cond->subtree = pending;
				}
				break;
			}

			case NetqreExpType::PREDICATE_SET:
			{
				switch(pending[0]->name[0])
				{
					case '&':
					cur->bool_type = BoolOpType::AND;
					cur->subtree.push_back(pending[1]);
					cur->subtree.push_back(pending[2]);
					break;

					case '|':
					cur->bool_type = BoolOpType::OR;
					cur->subtree.push_back(pending[1]);
					cur->subtree.push_back(pending[2]);
					break;

					default:
					cur->bool_type = BoolOpType::NONE;
					cur->subtree.push_back(pending[0]);
					break;
				}
				break;
			}

			case NetqreExpType::PREDICATE:
			{
				cur->subtree.push_back(pending[0]);
				cur->subtree.push_back(pending[2]);
				switch(pending[1]->name[0])
				{
					case '=':
					cur->pred_type = PredOpType::EQUAL;
					break;

					case '-':
					cur->pred_type = PredOpType::IN;
					break;

					case '>':
					cur->pred_type = PredOpType::BIGGER;
					break;

					case '<':
					cur->pred_type = PredOpType::SMALLER;
					break;
				}
				break;
			}

			case NetqreExpType::FEATURE_NI:
			{
				cur->value = cur->name.to_i();
				break;
			}

			case NetqreExpType::VALUE:
			{
				cerr << " " << cur->name << "";
				cur->value = pending[0]->value;
				break;
			}

			case NetqreExpType::VALUE_DIGIT:
			{
				if (pending.size() != 0)
					cur->name = pending[0]->name + pending[1]->name;
				cerr << " " << cur->name << "";
				cur->value = Rubify::string("1" + cur->name).to_i(2);
				break;
			}

			case NetqreExpType::AGG_OP:
			{
				if (pending[0]->name == "max")
					cur->agg_type = AggOpType::MAX;
				else if (pending[0]->name == "min")
					cur->agg_type = AggOpType::MIN;
				else if (pending[0]->name == "sum")
					cur->agg_type = AggOpType::SUM;
				break;
			}

			case NetqreExpType::UNKNOWN:
			{
				cur->bool_type = BoolOpType::NONE;
				break;
			}

			case NetqreExpType::PENDING_LITERAL:
				break;

			default:
				break;
		}

		std::cerr<<" | "<<node_name<<" "<<cursor<<" "<<cur->subtree.size()<<" }";

		return {cur, cursor2};
	};
	auto [ast, cursor] = parse_node(0, parse_node);
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
