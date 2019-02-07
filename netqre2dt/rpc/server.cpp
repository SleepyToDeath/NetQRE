#include <rpc/server.h>
#include <iostream>
#include "client.hpp"

int main() {
	rpc::server srv(23333);
	srv.bind("wtf", [](std::string code) {
		std::cout<<"#1\n";
		Netqre::IntValueMsg ret;
		ret.lower = code[0]-'0';
		ret.upper = code[1]-'0';
		for (int i=0; i<100000; i++);
		std::cout<<"#2\n";
		return ret;
	});

	constexpr size_t thread_count = 2;

	srv.async_run(thread_count);

	std::cin.ignore();

	return 0;
}

