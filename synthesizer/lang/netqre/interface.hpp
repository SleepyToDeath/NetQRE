#ifndef NETQRE_INTERFACE_HPP
#define NETQRE_INTERFACE_HPP

#include "../../../netqre2dt/rpc/client.hpp"
#include "../general/general.hpp"
#include "../../../netqre2dt/interpreter.h"
#include "../../../netqre2dt/parser.h"
#include "../../../netqre2dt/network_tokenizer/tcp_ip.hpp"
#include <sstream>
#include <algorithm>
#include <fstream>
#include <map>
#include <vector>

using std::min;
using std::max;
using std::ifstream;
using std::map;
using std::vector;
using std::max;
using std::string;

class NetqreExample: public GeneralExample {
	public:
	vector<TokenStream> positive_token;
	vector<TokenStream> negative_token;
};

class NetqreInterpreterInterface: public GeneralInterpreter {
	public:
	NetqreInterpreterInterface(vector<string> servers, vector<int> ports){
		manager = new Netqre::NetqreClientManager(servers, ports);
	}

	GeneralMatchingResult accept(AbstractCode code, bool complete,  shared_ptr<GeneralExample> input, IEConfig cfg) {
		auto e = std::static_pointer_cast<NetqreExample>(input);
//		auto ast = parser.parse(code.pos);
//		auto m = interpreter.interpret(ast);
		GeneralMatchingResult res;


		StreamFieldType pos_min_upper = -1;
		StreamFieldType pos_min_lower = -1;
		StreamFieldType neg_max_upper = -1;
		StreamFieldType neg_max_lower = -1;

		vector<std::unique_ptr<Netqre::IntValue> > ans_pos;
		vector<std::unique_ptr<Netqre::IntValue> > ans_neg;

		manager->exec(code.pos, e->positive_token.size(), e->negative_token.size(), ans_pos, ans_neg);

		for (int i=0; i<e->positive_token.size(); i++)
		{
//			auto s = e->positive_token[i];
//			auto ans = m->process(s);
			unique_ptr<Netqre::IntValue>& ans = ans_pos[i];
			if (ans->lower < 0)
			{
				res.accept = false;
				res.utility_rate = 1.0;
				return res;
			}
			if (i==0)
			{
				pos_min_upper = ans->upper;
				pos_min_lower = ans->lower;
			}
			else
			{
				pos_min_upper = min(pos_min_upper, ans->upper);
				pos_min_lower = min(pos_min_lower, ans->lower);
			}
//			ans_pos.push_back(std::move(ans));
		}

		for (int i=0; i<e->negative_token.size(); i++)
		{
//			auto s = e->negative_token[i];
//			auto ans = m->process(s);
			unique_ptr<Netqre::IntValue>& ans = ans_neg[i];
			if (ans->lower < 0)
			{
				res.accept = false;
				res.utility_rate = 1.0;
				return res;
			}
			if (i==0)
			{
				neg_max_upper = ans->upper;
				neg_max_lower = ans->lower;
			}
			else
			{
				neg_max_upper = max(neg_max_upper, ans->upper);
				neg_max_lower = max(neg_max_lower, ans->lower);
			}
//			ans_neg.push_back(std::move(ans));
		}

		if (!complete)
		{
			res.accept = neg_max_lower < pos_min_upper;
//			cout<<"[output]:"<<neg_max_lower<<" ~ "<<neg_max_upper<<"     ~     "<<pos_min_lower<<" ~ "<<pos_min_upper<<endl;
		}
		else
		{
			res.accept = 	(neg_max_upper < pos_min_lower) 
							|| 
					 		(
								(neg_max_upper == pos_min_lower) 
								&& 
								(
									(neg_max_lower != neg_max_upper) 
									|| 
									(pos_min_lower != pos_min_upper)
								)
							);
			if (res.accept)
			{
				cout<<"Answer found:"<<code.pos<<endl<<" Threshold range:"<<neg_max_upper<<" ~ "<<pos_min_lower<<endl;
				for (int i=0; i<ans_pos.size(); i++)
					cout<<"Positive output#"<<i<<": "<<ans_pos[i]->lower<<"~"<<ans_pos[i]->upper<<endl;
				for (int i=0; i<ans_neg.size(); i++)
					cout<<"Negative output#"<<i<<": "<<ans_neg[i]->lower<<"~"<<ans_neg[i]->upper<<endl;
			}
		}

		res.utility_rate = 1.0;

		return res;
	}

	double extra_complexity(AbstractCode code) {
		return 0.0;
	}

	vector<string> get_range(int handle, shared_ptr<GeneralExample> input)
	{
		auto e = std::static_pointer_cast<NetqreExample>(input);

		if  (handle == 0)
		{
			vector<string> ans;
			for(int i=0; i<e->positive_token[0][0].size(); i++)
			{
				stringstream ss;
				string tmp;
				ss<<i;
				ss>>tmp;
				ans.push_back(tmp);
			}
			return ans;
		}
		else
		{
			handle--;
		}

		vector<StreamFieldType> range_raw;
		vector<StreamFieldType> range;

		/* collect */
		for (int i=0; i<e->positive_token.size(); i++)
			for (int j=0; j<e->positive_token[i].size(); j++)
				range_raw.push_back(e->positive_token[i][j][handle].value);
		for (int i=0; i<e->negative_token.size(); i++)
			for (int j=0; j<e->negative_token[i].size(); j++)
				range_raw.push_back(e->negative_token[i][j][handle].value);

		/* remove dup */
		std::sort(range_raw.begin(), range_raw.end());
		range.push_back(range_raw[0]);
		for (int i=1; i<range_raw.size(); i++)
			if (range_raw[i] != range_raw[i-1])
				range.push_back(range_raw[i]);

		/* convert to string */
		vector<string> ans;
		for (int i=0; i<range.size(); i++)
		{
			stringstream ss;
			string tmp;
			ss<<range[i];
			ss>>tmp;
			cout<<"range+= "<<tmp<<endl;
			ans.push_back(tmp);
		}

		return ans;
	}

	Netqre::Interpreter interpreter;
	Netqre::NetqreParser parser;
	Netqre::NetqreClientManager* manager;
};

shared_ptr<NetqreExample> prepare_examples(ifstream& fin) {
	auto res = shared_ptr<NetqreExample>(new NetqreExample());
	int pos_n, neg_n, width;
	fin>>pos_n>>neg_n>>width;
	for (int i=0; i<pos_n; i++)
	{
		int len;
		fin>>len;
		TokenStream stream;
		for (int j=0; j<len; j++)
		{
			string pkt;
			fin>>pkt;
			FeatureVector token;
			for (int k=0; k<width; k++)
			{
				FeatureSlot digit(32, true, pkt[k]-'0');
				token.push_back(digit);
			}
			stream.push_back(token);
		}
		res->positive_token.push_back(stream);
	}

	for (int i=0; i<neg_n; i++)
	{
		int len;
		fin>>len;
		TokenStream stream;
		for (int j=0; j<len; j++)
		{
			string pkt;
			fin>>pkt;
			FeatureVector token;
			for (int k=0; k<width; k++)
			{
				FeatureSlot digit(32, true, pkt[k]-'0');
				token.push_back(digit);
			}
			stream.push_back(token);
		}
		res->negative_token.push_back(stream);
	}

	return res;
}

shared_ptr<NetqreExample> prepare_examples_old() {
	TokenStream pos;
	TokenStream neg1;
	TokenStream neg2;
	FeatureSlot zero(32, true, 0);
	FeatureSlot one(32, true, 1);
	FeatureVector v00;
	FeatureVector v01;
	FeatureVector v10;
	FeatureVector v11;
	v00.push_back(zero); v00.push_back(zero);
	v01.push_back(zero); v01.push_back(one);
	v10.push_back(one); v10.push_back(zero);
	v11.push_back(one); v11.push_back(one);
	pos.push_back(v01);
	pos.push_back(v11);
	pos.push_back(v01);
	pos.push_back(v11);
	pos.push_back(v00);
	pos.push_back(v10);
	pos.push_back(v01);
	pos.push_back(v11);
	pos.push_back(v01);
	pos.push_back(v11);
	pos.push_back(v00);
	pos.push_back(v10);
	pos.push_back(v01);
	pos.push_back(v01);
	pos.push_back(v00);

	neg1.push_back(v01);
	neg1.push_back(v01);
	neg1.push_back(v00);

	neg2.push_back(v01);
	neg2.push_back(v01);
	neg2.push_back(v00);
	neg2.push_back(v01);
	neg2.push_back(v01);
	neg2.push_back(v00);
	neg2.push_back(v01);
	neg2.push_back(v01);
	neg2.push_back(v00);
	neg2.push_back(v01);
	neg2.push_back(v01);
	neg2.push_back(v00);
	neg2.push_back(v01);
	neg2.push_back(v01);
	neg2.push_back(v00);
	neg2.push_back(v01);
	neg2.push_back(v01);
	neg2.push_back(v00);
	neg2.push_back(v01);
	neg2.push_back(v01);
	neg2.push_back(v00);

	auto ans = shared_ptr<NetqreExample>(new NetqreExample());
	ans->positive_token.push_back(pos);
	ans->negative_token.push_back(neg1);
	ans->negative_token.push_back(neg2);
	return ans;
}

shared_ptr<NetqreExample> prepare_examples_from_pcap(string positive_file_name, string negative_file_name, int threshold) {
	the_tcp_ip_parser = shared_ptr<TcpIpParser>(new TcpIpParser());
	auto examples = shared_ptr<NetqreExample>(new NetqreExample());

	{
		auto raw_stream = the_tcp_ip_parser->parse_pcap(negative_file_name, true);
		map<vector<int>, TokenStream > flows;
		for (int i=0; i<raw_stream->size(); i++)
		{
			vector<int> tuple;
			tuple.push_back((*raw_stream)[i][0].value);
			tuple.push_back((*raw_stream)[i][1].value);
			tuple.push_back((*raw_stream)[i][2].value);
			tuple.push_back((*raw_stream)[i][3].value);

			if ((tuple[0] > tuple[1]) || (tuple[0] == tuple[1] && tuple[2]>tuple[3]))
			{
				int tmp = tuple[0];
				tuple[0] = tuple[1];
				tuple[1] = tmp;
				tmp = tuple[2];
				tuple[2] = tuple[3];
				tuple[3] = tmp;
			}

			FeatureVector cur;
			cur.push_back((*raw_stream)[i][4]);
			cur.push_back((*raw_stream)[i][5]);

			flows[tuple].push_back(cur);
		}

		size_t maximum = 0;
		size_t count = 0; 
		vector<TokenStream> raw_negative;
		for_each(flows.begin(), flows.end(), [&](std::pair<vector<int>, TokenStream> cur) {
			if (cur.second.size()>threshold)
			{
				raw_negative.push_back(cur.second);
				count ++;
				maximum = max(maximum, cur.second.size());
			}
		});
		cout<<"Totally ["<<count<<"] flows longer than threshold. Maximum length is ["<<maximum<<"].\n";

		
		for (int i=0; i<raw_negative.size(); i++)
		{
			TokenStream& flow = raw_negative[i];
			FeatureVector pkt;
			bool valid = false;
			int step = 0;
			for (int j=0; j<flow.size(); j++)
			{
				pkt = flow[j];
				if (pkt[0].value == 1 && pkt[1].value == 0)
					step = 1;
				else if (step == 1 && pkt[0].value == 1 && pkt[1].value == 1)
					step++;
				else if (step == 2 && pkt[0].value == 0 && pkt[1].value == 1)
				{
					valid = true;
					break;
				}
				else
					step = 0;

			}
			if (!valid)
				examples->negative_token.push_back(flow);
		}

		cout<<"["<<examples->negative_token.size()<<"] negative flows left\n";

	}


	{
		auto raw_stream = the_tcp_ip_parser->parse_pcap(positive_file_name, false);
		map<vector<int>, TokenStream > flows;
		for (int i=0; i<raw_stream->size(); i++)
		{
			vector<int> tuple;
			tuple.push_back((*raw_stream)[i][0].value);
			tuple.push_back((*raw_stream)[i][1].value);
			tuple.push_back((*raw_stream)[i][2].value);
			tuple.push_back((*raw_stream)[i][3].value);

			if ((tuple[0] > tuple[1]) || (tuple[0] == tuple[1] && tuple[2]>tuple[3]))
			{
				int tmp = tuple[0];
				tuple[0] = tuple[1];
				tuple[1] = tmp;
				tmp = tuple[2];
				tuple[2] = tuple[3];
				tuple[3] = tmp;
			}

			FeatureVector cur;
			cur.push_back((*raw_stream)[i][4]);
			cur.push_back((*raw_stream)[i][5]);

			flows[tuple].push_back(cur);
		}

		size_t maximum = 0;
		size_t count = 0; 
		vector<TokenStream> raw_positive;
		for_each(flows.begin(), flows.end(), [&](std::pair<vector<int>, TokenStream> cur) {
			if (cur.second.size()>threshold)
			{
				raw_positive.push_back(cur.second);
				count ++;
				maximum = max(maximum, cur.second.size());
			}
		});
		cout<<"Totally ["<<count<<"] flows longer than threshold. Maximum length is ["<<maximum<<"].\n";

		
		for (int i=0; i<raw_positive.size(); i++)
		{
			TokenStream& flow = raw_positive[i];
			FeatureVector pkt;
			bool valid = false;
			int step = 0;
			for (int j=0; j<flow.size(); j++)
			{
				pkt = flow[j];
				if (pkt[0].value == 1 && pkt[1].value == 0)
				{
					step = 1;
//					cout<<"step1\n";
				}
				else if (step == 1 && pkt[0].value == 1 && pkt[1].value == 1)
				{
					step++;
//					cout<<"step2\n";
				}
				else if (step == 2 && pkt[0].value == 0 && pkt[1].value == 1)
				{
//					cout<<"step3\n";
					valid = true;
					break;
				}
				else
					step = 0;
			}
			if (valid)
				examples->positive_token.push_back(flow);
		}

		cout<<"["<<examples->positive_token.size()<<"] positive flows left\n";

	}

/*
	for (int i=0; i<examples->negative_token.size(); i++)
	{
		for (int j=0; j<examples->negative_token[i].size(); j++)
			cout<<examples->negative_token[i][j][0].value<<examples->negative_token[i][j][1].value<<"-";
		cout<<endl;
	}
	*/

	return examples;
}

#endif

