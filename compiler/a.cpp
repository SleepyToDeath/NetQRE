// leaf
struct Node_re0_leaf{
int state_re0;
};
// y
struct Node_re0_y{
unordered_map<int, Node_re0_leaf> stateMap;
Node_re0_leaf default_state;
};
// x
struct Node_re0_x{
unordered_map<int, Node_re0_y> stateMap;
Node_re0_y default_state;
};
bool hh_update(Packet *last) {
unordered_map<int, Node_re0_y>::iterator it_x;
Node_re0_y *node_y;
unordered_map<int, Node_re0_leaf>::iterator it_y;
Node_re0_leaf *node_leaf;


it_x = node_x->state_map.find(src);
if (it_x == node_x->state_map.end()) { 
it_x = node_x->state_map.insert({src, node_x->default_state}).first;
}
if (true) {
node_y = &(it_x->second);

it_y = node_y->state_map.find(dst);
if (it_y == node_y->state_map.end()) { 
it_y = node_y->state_map.insert({dst, node_y->default_state}).first;
}
if (true) {
node_leaf = &(it_y->second);

switch (node_leaf->state_re0) {
case 0: 
node_leaf->state_re0 = 1;
break;
case 1: 
node_leaf->state_re0 = 1;
break;
default:
node_leaf->state_re0 = -1;
break;
}
}
}
return true;
}

int hh_eval(ip x, ip y, Packet* last) {
    unordered_map<int, Node_re0_y>::iterator it_x;
    Node_re0_y *node_y;
    unordered_map<int, Node_re0_leaf>::iterator it_y;
    Node_re0_leaf *node_leaf;


    it_x = node_x->state_map.find(x);
    if (it_x == node_x->state_map.end()) { 
	it_x = node_x->default_state;
    }
    node_y = &(it_x->second);

    it_y = node_y->state_map.find(y);
    if (it_y == node_y->state_map.end()) { 
	it_y = node_y->default_state;
    }
    node_leaf = &(it_y->second);

    switch (node_leaf->state_re0) {
	case 1: 
	    ret_re0 = true;
	    break;
	default:
	    ret_re0 = false;
    }
    int ret_ = 0;
    if (ret_re0)
	ret_ == 1;
    return ret_
}

