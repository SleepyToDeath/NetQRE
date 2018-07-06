#include <iostream>
#include <experimental/random>

using namespace std;

void xlarge3()
{
	cout<<10<<endl;
	cout<<5<<endl;
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
	cout<<1<<endl;
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

	int len0 = 5;



	cout<<10000<<endl;
	cout<<2<<endl;
	int upper = len0*11/10;
	int lower = len0*9/10;
	for (int i=0; i<2; i++)
	{
		int len = experimental::randint(lower,upper);
		for (int j=0; j<len; j++)
			cout<<experimental::randint(0,1);
		cout<<endl;
	}
	cout<<3<<endl;
	for (int i=0; i<3; i++)
	{
		int len = experimental::randint(lower,upper);
		for (int j=0; j<len; j++)
			cout<<experimental::randint(0,1);
		cout<<endl;
	}
}

int main() {
//	random_small();
	xlarge3();
}
