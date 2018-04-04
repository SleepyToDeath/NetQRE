// leaf
struct Node_re0_leaf{
    int state_re0;
};
// y
struct Node_re0_y{
    unordered_map<int, Node_re0_leaf> stateMap;
    Node_re0_leaf default_state;
};
bool getFirst_update(Packet *last) {
    unordered_map<int, Node_re0_leaf>::iterator it_y;
    Node_re0_leaf *node_leaf;

    it_y = node_y->state_map.find(dst);
    node_leaf = &(it_y->second);

    if (it_y == node_y->state_map.end()) { 
	Node_re0_leaf node = node_y->default_state;
	node_leaf = &(node);
    }
    if (true) {
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
    node_leaf = &(node_y->default_state);

    switch (node_leaf->state_re0) {
	case 1: 
	    node_leaf->state_re0 = 1;
	    break;
	default:
	    node_leaf->state_re0 = -1;
	    break;
    }
    for (it_y = node_y->state_map.begin(); it_y != node_y->state_map.end(); it_y++) {
	if (true && it_y->first != dst) {
	    node_leaf = &(it_y->second);

	    switch (node_leaf->state_re0) {
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

