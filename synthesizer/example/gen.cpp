#include <iostream>
#include <vector>
#include <experimental/random>

using namespace std;

void xlarge3()
{
	cout<<"5 1"<<endl;
	for (int k=0; k<5; k++)
	{
		for (int i=0; i<4; i++)
		{
			int l = experimental::randint(50,100);
			for (int j=0; j<l; j++)
			{
				cout<<0;
			}
			if (i<3)
			{
				cout<<1;
				cout<<1;
			}
			if (i<2)
			{
				cout<<1;
			}
			if (i<1)
			{
				cout<<1;
			}
		}
		cout<<endl;
	}
//	cout<<0<<endl;
	for (int i=0; i<4; i++)
	{
		int l = experimental::randint(50,100);
		for (int j=0; j<l; j++)
		{
			cout<<0;
		}
		if (i<3)
		{
			cout<<1;
			cout<<1;
			cout<<1;
		}
	}
	cout<<endl;
}

void random_small() {

	int len0 = 20;
	int sizep = 3;
	int sizen = 2;
	cout<<sizep<<" "<<sizen<<endl;
	int upper = len0*11/10;
	int lower = len0*9/10;
	for (int i=0; i<sizep; i++)
	{
		int len = experimental::randint(lower,upper);
		for (int j=0; j<len; j++)
			cout<<experimental::randint(0,1);
		cout<<endl;
	}
	for (int i=0; i<sizen; i++)
	{
		int len = experimental::randint(lower,upper);
		for (int j=0; j<len; j++)
			cout<<experimental::randint(0,1);
		cout<<endl;
	}
}

bool accept_passwd(string passwd)
{
	bool flag_len = false;
	bool flag_d = false;
	bool flag_c = false;
	bool flag_C = false;
	int threshold = 8;

	for (char c='A'; c<='Z'; c++)
		if (string::npos != passwd.find(c))
		{
			flag_C = true;
			break;
		}
	for (char c='a'; c<='z'; c++)
		if (string::npos != passwd.find(c))
		{
			flag_c = true;
			break;
		}
	for (char c='0'; c<='9'; c++)
		if (string::npos != passwd.find(c))
		{
			flag_d = true;
			break;
		}

	return flag_d || flag_C;
}

void passwd()
{
	int num = 20;
	int lower = 3;
	int upper = 30;

	vector<char> char_set;
	for (char c='a'; c<='z'; c++)
		char_set.push_back(c);
	for (char c='A'; c<='B'; c++)
		char_set.push_back(c);
	for (char c='0'; c<='1'; c++)
		char_set.push_back(c);

	vector<string> pos;
	vector<string> neg;

	for (int i=0; i<num; i++)
	{
		int len = experimental::randint(lower,upper);
		string s;
		for (int j=0; j<len; j++)
		{
			char next = char_set[experimental::randint(0, (int)char_set.size()-1)];
			s.append(1, next);
		}
		if (accept_passwd(s))
			pos.push_back(s);
		else
			neg.push_back(s);
	}

	cout<<pos.size()<<" "<<neg.size()<<endl;
	for (int i=0; i<pos.size(); i++)
		cout<<pos[i]<<endl;
	for (int i=0; i<neg.size(); i++)
		cout<<neg[i]<<endl;
}

void int_threshold() {
	int threshold = 256;
	vector<int> pos;
	vector<int> neg;
	int lower = 1;
	int upper = 1000;
	for (int i=0; i<20; i++)
	{
		int n = experimental::randint(lower,upper);
		if (n<threshold)
			pos.push_back(n);
		else
			neg.push_back(n);
	}
	cout<<pos.size()<<" "<<neg.size()<<endl;
	for (int i=0; i<pos.size(); i++)
		cout<<pos[i]<<endl;
	for (int i=0; i<neg.size(); i++)
		cout<<neg[i]<<endl;
}

int main() {
//	random_small();
//	xlarge3();
	passwd();
//	int_threshold();
}
