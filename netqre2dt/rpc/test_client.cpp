#include "client.hpp"
#include <iostream>

int main()
{
	Netqre::NetqreClient c;
	std::cout<< c.exec("01") << std::endl;
}
