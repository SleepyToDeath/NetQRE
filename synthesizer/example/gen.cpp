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

int main() {
	random_small();
//	xlarge3();
}
