#include "parser.h"
#include "interpreter.h"
#include <iostream>

using namespace std;

TokenStream generate_test_case()
{
	TokenStream s;
	FeatureSlot zero(32, true, 1);
	FeatureSlot one(32, true, 0);
	FeatureVector v00;
	FeatureVector v01;
	FeatureVector v10;
	FeatureVector v11;
	v00.push_back(zero); v00.push_back(zero);
	v01.push_back(zero); v01.push_back(one);
	v10.push_back(one); v10.push_back(zero);
	v11.push_back(one); v11.push_back(one);
	s.push_back(v01);
	s.push_back(v11);
	s.push_back(v01);
	s.push_back(v11);
	s.push_back(v00);
	s.push_back(v10);
	s.push_back(v01);
	s.push_back(v11);
	s.push_back(v01);
	s.push_back(v11);
	s.push_back(v00);
	s.push_back(v10);
	s.push_back(v01);
	s.push_back(v01);
	s.push_back(v00);
	return s;
}

int main()
{
	string code;
	getline(cin, code);
	for (int i=0; i<1; i++)
	{
		try
		{
		Netqre::NetqreParser parser;
		Netqre::Interpreter itp;
		auto ast = parser.parse(code);
		auto m = itp.interpret(ast);
		auto input = generate_test_case();
		auto ans = m->process(input);
		cout<<"Result #"<<i<<": "<< ans->lower << " ~ " << ans->upper <<endl;
		auto mm = m;
		}
		catch (string s)
		{
			cout<<s;
		}
	}
}
