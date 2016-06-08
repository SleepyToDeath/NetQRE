// leaf
struct Node__leaf{
    int state_re0;
    int state_re1;
};
// x
struct Node__x{
    unordered_map<int, Node__leaf> stateMap;
    Node__leaf default_state;
};
bool hh_update(Packet *last) {
    unordered_map<int, Node__leaf>::iterator it_x;
    Node__leaf *node_leaf;

    it_x = node_x->state_map.find(src);
    if (it_x == node_x->state_map.end()) { 
	it_x = node_x->state_map.insert({src, node_x->default_state}).first;
    }
    if (true) {
	node_leaf = &(it_x->second);

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
    node_leaf = &(node_x->default_state);

    switch (node_leaf->state_re0) {
	case 0: 
	    node_leaf->state_re0 = 0;
	    break;
	case 1: 
	    node_leaf->state_re0 = 0;
	    break;
	default:
	    node_leaf->state_re0 = -1;
	    break;
    }
    for (it_x = node_x->state_map.begin(); it_x != node_x->state_map.end(); it_x++) {
	if (true && it_x->first != src) {
	    node_leaf = &(it_x->second);

	    switch (node_leaf->state_re0) {
		case 0: 
		    node_leaf->state_re0 = 0;
		    break;
		case 1: 
		    node_leaf->state_re0 = 0;
		    break;
		default:
		    node_leaf->state_re0 = -1;
		    break;
	    }
	}
    }
    // In 
    for (it_x=node_x->state_map.begin(); it_x!=node_x->state_map.end();it_x++) {
	node_leaf = &(it_x->second);

	// eval for left
	switch (node_leaf->state_re0) {
	    case 1: 
		ret_re0 = true;
		break;
	    default:
		ret_re0 = false;
	}
	int ret_ = 0;
	if (ret_re0) {
	    ret_ == ;
	}
	// update for right
	if (it_x->first == src) {
	    node_leaf = &(it_x->second);

	    switch (node_leaf->state_re1) {
		case 0: 
		    node_leaf->state_re1 = 1;
		    break;
		case 1: 
		    node_leaf->state_re1 = 1;
		    break;
		default:
		    node_leaf->state_re1 = -1;
		    break;
	    }
	}
	else {
	}
    }
    node_leaf = &(node_x->default_state);

    // eval for left
    switch (node_leaf->state_re0) {
	case 1: 
	    ret_re0 = true;
	    break;
	default:
	    ret_re0 = false;
    }
    int ret_ = 0;
    if (ret_re0) {
	ret_ == ;
    }
    // update for right
    if (node_x->state_map.find(src) == node_x->state_map.end()) {
	node_leaf = &(it_x->second);

	switch (node_leaf->state_re1) {
	    case 0: 
		node_leaf->state_re1 = 1;
		break;
	    case 1: 
		node_leaf->state_re1 = 1;
		break;
	    default:
		node_leaf->state_re1 = -1;
		break;
	}
    }
    else {
    }
    return true;
}

int hh_eval(Packet* last) {
    unordered_map<int, Node__leaf>::iterator it_x;
    Node__leaf *node_leaf;

    return ret_;
}

