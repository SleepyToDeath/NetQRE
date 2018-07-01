#include <iostream>
#include <experimental/random>

using namespace std;

int main()
{
	for (int k=0; k<100; k++)
	{
		for (int i=0; i<4; i++)
		{
			int l = experimental::randint(2,10);
			for (int j=0; j<l; j++)
			{
				cout<<0;
			}
			if (i<3)
				cout<<1;
		}
		cout<<endl;
	}
}
