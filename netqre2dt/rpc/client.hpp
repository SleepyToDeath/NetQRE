#ifndef NETQRE_CLIENT_HPP
#define NETQRE_CLIENT_HPP

#include <rpc/client.h>
#include <mutex>
#include <iostream>
#include "../op.hpp"

using std::vector;
using std::unique_ptr;
using std::shared_ptr;
using std::future;
using std::cout;
using std::endl;
using std::mutex;

namespace Netqre {

constexpr int SERVICE_PORT = 23333;
constexpr int THREAD_COUNT = 4;
constexpr int MAX_WORKLOAD = 16;
const string SERVICE_NAME = string("netqre_service");

class IntValueMsg {
	public:
	StreamFieldType lower;
	StreamFieldType upper;
	MSGPACK_DEFINE_ARRAY(lower, upper)
};

class NetqreClient {
	public:
	NetqreClient(std::string server_name, int port):
	c(server_name, port)
	{
		buzy_threads = 0;
	}

	~NetqreClient()
	{
	}

	std::future< RPCLIB_MSGPACK::object_handle > exec(std::string code, bool example_positive, int example_index)
	{
		count_lock.lock();
		auto handle = c.async_call(SERVICE_NAME, code, example_positive, example_index);
		buzy_threads++;
		count_lock.unlock();
		return handle;
	}

	unique_ptr<IntValue> collect(std::future< RPCLIB_MSGPACK::object_handle >& handle)
	{
		count_lock.lock();
		IntValueMsg msg;
		msg = handle.get().as<IntValueMsg>();
		auto ans = unique_ptr<IntValue>(new IntValue());
		ans->upper = msg.upper;
		ans->lower = msg.lower;
		buzy_threads--;
		count_lock.unlock();
		return ans;
	}

	int get_buzy_threads()
	{
		count_lock.lock();
		int ret = buzy_threads;
		count_lock.unlock();
		return ret;
	}

	int buzy_threads;
	mutex count_lock;
	rpc::client c;
};

class NetqreClientManager {
	public:

	NetqreClientManager(vector<string> server_list)
	{
		for (int i=0; i<server_list.size(); i++)
			for (int j=0; j<32; j++)
				clients.push_back(new NetqreClient(server_list[i], SERVICE_PORT + j));
	}

	/* distributed execution, with load balancing, collect answers in pos_ans and neg_ans */
	/* examples should be already shared with servers */
	void exec(string code, int pos_size, int neg_size, vector<unique_ptr<IntValue> >& pos_ans, vector<unique_ptr<IntValue> >& neg_ans)
	{
		vector<std::future< RPCLIB_MSGPACK::object_handle > > pos_handle;
		vector<std::future< RPCLIB_MSGPACK::object_handle > > neg_handle;
		vector<int> pos_client;
		vector<int> neg_client;

		for (int i=0; i<pos_size; i++)
		{
			int min_count = MAX_WORKLOAD + 1;
			int lazy_client = 0;
//			while(min_count > MAX_WORKLOAD)
			{
				min_count = clients[0]->get_buzy_threads();
				lazy_client = 0;
				for (int j=1; j<clients.size(); j++)
				{
					int tmp = clients[j]->get_buzy_threads();
					if (tmp < min_count)
					{
						min_count = tmp;
						lazy_client = j;
					}
				}
			}
			pos_handle.push_back(clients[lazy_client]->exec(code, true, i));
			pos_client.push_back(lazy_client);
		}

		for (int i=0; i<neg_size; i++)
		{
			int min_count = MAX_WORKLOAD + 1;
			int lazy_client = 0;
//			while(min_count > MAX_WORKLOAD)
			{
				min_count = clients[0]->get_buzy_threads();
				lazy_client = 0;
				for (int j=1; j<clients.size(); j++)
				{
					int tmp = clients[j]->get_buzy_threads();
					if (tmp < min_count)
					{
						min_count = tmp;
						lazy_client = j;
					}
				}
			}
			neg_handle.push_back(clients[lazy_client]->exec(code, false, i));
			neg_client.push_back(lazy_client);
		}

		for (int i=0; i<pos_handle.size(); i++)
			pos_ans.push_back(clients[ pos_client[i] ]->collect( pos_handle[i] ));

		for (int i=0; i<neg_handle.size(); i++)
			neg_ans.push_back(clients[ neg_client[i] ]->collect( neg_handle[i] ));

	}

	vector<NetqreClient*> clients;
};

}

#endif
