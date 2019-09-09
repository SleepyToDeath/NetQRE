#include "client.hpp"

namespace Netqre {

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

};


