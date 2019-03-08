#ifndef NETQRE_CLIENT_HPP
#define NETQRE_CLIENT_HPP

#include <rpc/client.h>
#include <mutex>
#include <iostream>
#include "../op.hpp"
#include <utility>
#include <map>

using std::vector;
using std::unique_ptr;
using std::shared_ptr;
using std::future;
using std::cout;
using std::endl;
using std::mutex;
using std::map;
using std::pair;

namespace Netqre {

constexpr int THREAD_COUNT = 4;
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

typedef std::future< RPCLIB_MSGPACK::object_handle > RpcHandle;

class ExecConf
{
	public:
	string code;
	bool pos;
	int index;
};

bool operator <(const ExecConf a, const ExecConf b)
{
	if (a.code.size() < b.code.size())
		return true;
	if (a.code.size() > b.code.size())
		return false;
	for (int i=0; i<a.code.size(); i++)
	{
		if (a.code[i] < b.code[i])
			return true;
		if (a.code[i] > b.code[i])
			return false;
	}
	if (a.pos && !b.pos)
		return true;
	if (b.pos && !a.pos)
		return false;
	if (a.index < b.index)
		return true;
	if (b.index < a.index)
		return false;
	return false;
}


class NetqreClientManager {
	public:

	NetqreClientManager(vector<string> server_list, vector<int> port_list)
	{
		if (server_list.size() != port_list.size())
			throw string("Servers and ports don't match!\n");
		for (int i=0; i<server_list.size(); i++)
			clients.push_back(new NetqreClient(server_list[i], port_list[i]));
		cache.clear();
	}

	/* distributed execution, with load balancing, collect answers in pos_ans and neg_ans */
	/* examples should be already shared with servers */
	void exec(string code, int pos_offset, int pos_size, int neg_offset, int neg_size, vector<unique_ptr<IntValue> >& pos_ans, vector<unique_ptr<IntValue> >& neg_ans)
	{
		vector< RpcHandle > pos_handle;
		vector< RpcHandle > neg_handle;
		vector<int> pos_client;
		vector<int> neg_client;

		vector<ExecConf> pos_conf_list;
		vector<ExecConf> neg_conf_list;

		for (int i=pos_offset; i<pos_offset+pos_size; i++)
		{
			ExecConf conf;
			conf.code = code;
			conf.pos = true;
			conf.index = i;
			pos_conf_list.push_back(conf);
			cache_lock.lock();
			if (cache.count(conf) > 0)
			{
				pos_handle.push_back(RpcHandle());
				pos_client.push_back(-1);
				cache_lock.unlock();
				continue;
			}
			cache_lock.unlock();

			int lazy_client = next_client;
			lazy_client++;
			if (lazy_client >= clients.size())
				lazy_client = 0;
			next_client = lazy_client;
			pos_handle.push_back(clients[lazy_client]->exec(code, true, i));
			pos_client.push_back(lazy_client);
		}

		for (int i=neg_offset; i<neg_offset+neg_size; i++)
		{
			ExecConf conf;
			conf.code = code;
			conf.pos = false;
			conf.index = i;
			neg_conf_list.push_back(conf);
			cache_lock.lock();
			if (cache.count(conf) > 0)
			{
				neg_handle.push_back(RpcHandle());
				neg_client.push_back(-1);
				cache_lock.unlock();
				continue;
			}
			cache_lock.unlock();

			int lazy_client = next_client;
			lazy_client++;
			if (lazy_client >= clients.size())
				lazy_client = 0;
			next_client = lazy_client;
			neg_handle.push_back(clients[lazy_client]->exec(code, false, i));
			neg_client.push_back(lazy_client);
		}

		for (int i=0; i<pos_handle.size(); i++)
			if (pos_client[i] != -1)
			{
				pos_ans.push_back(clients[ pos_client[i] ]->collect( pos_handle[i] ));
				cache_lock.lock();
				cache[pos_conf_list[i]] = copy_typed_data(IntValue, pos_ans.back());
				cache_lock.unlock();
			}
			else
			{
				cache_lock.lock();
				pos_ans.push_back(copy_typed_data(IntValue, cache[pos_conf_list[i]]));
				cache_lock.unlock();
			}

		for (int i=0; i<neg_handle.size(); i++)
			if (neg_client[i] != -1)
			{
				neg_ans.push_back(clients[ neg_client[i] ]->collect( neg_handle[i] ));
				cache_lock.lock();
				cache[neg_conf_list[i]] = copy_typed_data(IntValue, neg_ans.back());
				cache_lock.unlock();
			}
			else
			{
				cache_lock.lock();
				neg_ans.push_back(copy_typed_data(IntValue, cache[neg_conf_list[i]]));
				cache_lock.unlock();
			}

	}

	int next_client = 0;
	vector<NetqreClient*> clients;

	mutex cache_lock;
	map< ExecConf, unique_ptr<IntValue> > cache;
};

}

#endif
