#include <unordered_map>
#include <iostream>
#include <tuple>

using namespace std;

typedef int IP;

struct Packet {
    int src = 0;
    int dst = 0;
};
// leaf
struct Node_split0_leaf{
int state_re1 = 0;
};
// y
struct Node_split0_y{
int state_re0 = 0;
bool isSplit_split0 = false;
unordered_map<int, Node_split0_leaf> state_map;
Node_split0_leaf default_state;
};
// x
struct Node_split0_x{
unordered_map<int, Node_split0_y> state_map;
Node_split0_y default_state;
};
Node_split0_x *node_split0_x = new Node_split0_x();

bool main_update(Packet *last) {
unordered_map<int, Node_split0_y>::iterator it_split0_x;
Node_split0_y *node_split0_y;
unordered_map<int, Node_split0_leaf>::iterator it_split0_y;
Node_split0_leaf *node_split0_leaf;

// entering x
it_split0_x = node_split0_x->state_map.find(syn);
if (it_split0_x == node_split0_x->state_map.end()) { 
it_split0_x = node_split0_x->state_map.insert({syn, node_split0_x->default_state}).first;
}
if (true) {
node_split0_y = &(it_split0_x->second);

// entering y
if (!node_split0_y->isSplit_split0) {
switch (node_split0_y->state_re0) {
case 0: 
node_split0_y->state_re0 = 1;
node_split0_y->isSplit_split0 = true;
break;
default:
node_split0_y->state_re0 = -1;
break;
}
// leaving y
}
}
node_split0_y = &(node_split0_x->default_state);

// entering y
if (!node_split0_y->isSplit_split0) {
switch (node_split0_y->state_re0) {
default:
node_split0_y->state_re0 = -1;
break;
}
// leaving y
}
for (it_split0_x = node_split0_x->state_map.begin(); it_split0_x != node_split0_x->state_map.end(); it_split0_x++) {
if (true && it_split0_x->first != syn) {
node_split0_y = &(it_split0_x->second);

// entering y
if (!node_split0_y->isSplit_split0) {
switch (node_split0_y->state_re0) {
default:
node_split0_y->state_re0 = -1;
break;
}
// leaving y
}
}
}
// leaving x
// update for 2nd expr in split0
// entering x
node_split0_y = &(node_split0_x->default_state);

// entering y
if (node_split0_y->isSplit_split0) {
it_split0_y = node_split0_y->state_map.find(src);
if (it_split0_y == node_split0_y->state_map.end()) { 
it_split0_y = node_split0_y->state_map.insert({src, node_split0_y->default_state}).first;
}
node_split0_leaf = &(node_split0_y->default_state);

// entering leaf
switch (node_split0_leaf->state_re1) {
default:
node_split0_leaf->state_re1 = -1;
break;
}
// leaving leaf
for (it_split0_y = node_split0_y->state_map.begin(); it_split0_y != node_split0_y->state_map.end(); it_split0_y++) {
if (true && it_split0_y->first != src) {
node_split0_leaf = &(it_split0_y->second);

// entering leaf
switch (node_split0_leaf->state_re1) {
default:
node_split0_leaf->state_re1 = -1;
break;
}
// leaving leaf
}
}
// leaving y
}
for (it_split0_x = node_split0_x->state_map.begin(); it_split0_x != node_split0_x->state_map.end(); it_split0_x++) {
if (true) {
node_split0_y = &(it_split0_x->second);

// entering y
if (node_split0_y->isSplit_split0) {
if (it_split0_y->first == src) {
}
else {
node_split0_leaf = &(it_split0_y->second);

// entering leaf
switch (node_split0_leaf->state_re1) {
default:
node_split0_leaf->state_re1 = -1;
break;
}
// leaving leaf
}
// leaving y
}
}
}
// leaving x
return true;
}

int main_eval(Packet* last) {
unordered_map<int, Node_split0_y>::iterator it_split0_x;
Node_split0_y *node_split0_y;
unordered_map<int, Node_split0_leaf>::iterator it_split0_y;
Node_split0_leaf *node_split0_leaf;

return ;
}

Packet pkt;

int main() {
    while (true) {
	cout << "src = ";
	cin >> pkt.src;
	
	cout << "dst = ";
	cin >> pkt.dst;

	smain_update(&pkt);
	cout << "result is: \t" << smain_eval(&pkt) << endl;
    }
    return 1;
}
