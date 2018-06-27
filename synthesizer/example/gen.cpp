#include <iostream>

using namespace std;

int main()
{
	for (int i=0; i<200; i++)
	{
		for (int j=0; j<200; j++)
		{
			if (i == j)
				cout<<1;
			else
				cout<<0;
		}
		cout<<endl;
	}
}
