#ifndef NETQRE_CLIENT_HPP
#define NETQRE_CLIENT_HPP

#include <rpc/client.h>
#include "../op.hpp"

namespace Netqre {

class IntValueMsg {
	public:
	StreamFieldType lower;
	StreamFieldType upper;
	MSGPACK_DEFINE_ARRAY(lower, upper)
};

class NetqreClient {
	public:
	NetqreClient():
	c("127.0.0.1", 23333)
	{
	}

	int exec(std::string code)
	{
		IntValueMsg msg;
		std::vector<std::future< RPCLIB_MSGPACK::object_handle > > futures;
		for (int i=0; i<100; i++)
			futures.push_back(c.async_call("wtf", "01"));

		for (int i=0; i<100; i++)
			msg = futures[i].get().as<IntValueMsg>();

		return msg.upper;
	}

	rpc::client c;
};

}

#endif
