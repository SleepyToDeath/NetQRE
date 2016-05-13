// leaf
struct Node_agg0_leaf{
    int state_re0;
};
// x
struct Node_agg0_x{
    int sum_agg0;
    unordered_map<int, Node_agg0_leaf> stateMap;
    Node_agg0_leaf default_state;
};
bool hh_update(Packet *last) {
    unordered_map<int, Node_agg0_leaf>::iterator it_x;
    Node_agg0_leaf *node_leaf;

    it_x = node_x->state_map.find(src);
    if (it_x == node_x->state_map.end()) { 
	it_x = node_x->state_map.insert({src, node_x->default_state}).first;
    }
    node_leaf = &(it_x->second);

    switch (node_leaf->state_re0) {
	case 0: 
	    node_leaf->state_re0 = 1;
	    state.sum_0x212b870 += new - old;
	    break;
	case 1: 
	    node_leaf->state_re0 = 1;
	    state.sum_0x212b870 += new - old;
	    break;
	default:
	    node_leaf->state_re0 = -1;
	    break;
    }
    return true;
}

