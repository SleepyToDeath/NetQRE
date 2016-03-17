#ifndef _UTIL_H
#define _UTIL_H
#include <string>
#include <list>
using namespace std;

inline bool isIn(string var, list<string> &strList) {
    for (string str : strList) {
	if (str.compare(var) == 0)
	    return true;
    }
    return false;
}

inline void append(list<string>& first, const list<string>& second) {
    for (const string var : second) {
	bool exist = false;
	for (string var2 : first) {
	    if (var2.compare(var)==0) {
		exist = true;
		break;
	    }
	}
	if (!exist)
	    first.push_back(var);
    }
}

inline void printList(const list<string>& mylist) {
    for (const string var : mylist) {
	cout << var << " ";
    }
    cout << endl;
}

#endif
