#include <iostream>

using namespace std;

int main()
{
	for (char c='A'; c<='Z'; c++)
		cout<<"[ \""<<c<<"\" ], ";
	cout<<"[ \"\\C\" ]";
}
