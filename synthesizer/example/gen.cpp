#include <iostream>
#include <experimental/random>

using namespace std;

int main()
{
	cout<<10000<<endl;
	cout<<100<<endl;
	for (int k=0; k<100; k++)
	{
		for (int i=0; i<4; i++)
		{
			int l = experimental::randint(2,100);
			for (int j=0; j<l; j++)
			{
				cout<<0;
			}
			if (i<3)
				cout<<1;
		}
		cout<<endl;
	}
	cout<<0<<endl;
	/*
	cout<<1<<endl;
	for (int i=0; i<4; i++)
	{
		int l = experimental::randint(0,20);
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
	*/
}
