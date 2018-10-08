#include <iostream>

using namespace std;

int main()
{
	for (char c='0'; c<='9'; c++)
		cout<<"[ \""<<c<<"\" ], ";
	cout<<"[ \"\\d\" ]";
}
