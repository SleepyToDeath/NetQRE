#ifndef _UTIL_H
#define _UTIL_H
#include <string>
#include <list>
#include <sstream>
using namespace std;

#ifdef DEBUG_MODE
#define DEBUG(x)    x
#else
#define DEBUG(x)
#endif

inline bool isIn(string var, list<string> &strList) {
    for (string str : strList) {
	if (str.compare(var) == 0)
	    return true;
    }
    return false;
}

inline void mergeTo(list<string>& first, const list<string>& second) {
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

// append unique elements in second to first
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

inline bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

inline string toString(int n) {
    ostringstream convert;
    convert << n;
    return convert.str();
}

#endif
