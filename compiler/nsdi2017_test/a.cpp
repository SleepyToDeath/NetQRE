// leaf
struct Node_iter0_leaf{
  int sum_iter0 = 0;
  int state_re0 = 0;
};
Node_iter0_leaf *node_iter0_leaf = new Node_iter0_leaf();

// leaf
struct Node_iter1_leaf{
  int sum_iter1 = 0;
  int state_re1 = 0;
};
// x
struct Node_iter1_x{
  unordered_map<int, Node_iter1_leaf> state_map;
  Node_iter1_leaf default_state;
};
Node_iter1_x *node_iter1_x = new Node_iter1_x();

// x
struct Node_agg0_x{
  int sum_agg0 = 0;
};
Node_agg0_x *node_agg0_x = new Node_agg0_x();

bool total_update(Packet *last) {

  switch (node_iter0_leaf->state_re0) {
    case 0: 
      node_iter0_leaf->state_re0 = 1;
      node_iter0_leaf->sum_iter0 += 1;
      node_iter0_leaf->state_re0 = 0;
      break;
    default:
      node_iter0_leaf->state_re0 = -1;
      break;
  }
  return true;
}

bool count_src_update(Packet *last) {
  unordered_map<int, Node_iter1_leaf>::iterator it_iter1_x;
  Node_iter1_leaf *node_iter1_leaf;

  it_iter1_x = node_iter1_x->state_map.find(srcip);
  if (it_iter1_x == node_iter1_x->state_map.end()) { 
    it_iter1_x = node_iter1_x->state_map.insert({srcip, node_iter1_x->default_state}).first;
  }
  if (true) {
    node_iter1_leaf = &(it_iter1_x->second);

    switch (node_iter1_leaf->state_re1) {
      case 0: 
	node_iter1_leaf->state_re1 = 1;
	node_iter1_leaf->sum_iter1 += 1;
	node_iter1_leaf->state_re1 = 0;
	break;
      default:
	node_iter1_leaf->state_re1 = -1;
	break;
    }
  }
  node_iter1_leaf = &(node_iter1_x->default_state);

  switch (node_iter1_leaf->state_re1) {
    default:
      node_iter1_leaf->state_re1 = -1;
      break;
  }
  for (it_iter1_x = node_iter1_x->state_map.begin(); it_iter1_x != node_iter1_x->state_map.end(); it_iter1_x++) {
    if (true && it_iter1_x->first != srcip) {
      node_iter1_leaf = &(it_iter1_x->second);

      switch (node_iter1_leaf->state_re1) {
	default:
	  node_iter1_leaf->state_re1 = -1;
	  break;
      }
    }
  }
  return true;
}

bool frequency_update(Packet *last) {
  count_src_update(last);
  total_update(last);
  return true;
}

bool entropy_update(Packet *last) {
  unordered_map<int, Node_agg0_leaf>::iterator it_agg0_x;
  Node_agg0_leaf *node_agg0_leaf;

  frequency_update(last);
  return true;
}

int total_eval(Packet* last) {

  int ret_iter0 = 0;
  ret_iter0 = node_iter0_leaf->sum_iter0;
  return ret_iter0;
}

int count_src_eval(IP x, Packet* last) {
  unordered_map<int, Node_iter1_leaf>::iterator it_iter1_x;
  Node_iter1_leaf *node_iter1_leaf;

  it_iter1_x = node_iter1_x->state_map.find(x);
  if (it_iter1_x == node_iter1_x->state_map.end()) { 
    node_iter1_leaf = &node_iter1_x->default_state;
  } else {
    node_iter1_leaf = &(it_iter1_x->second);

  }
  int ret_iter1 = 0;
  ret_iter1 = node_iter1_leaf->sum_iter1;
  return ret_iter1;
}

int frequency_eval(IP x, Packet* last) {
  return ;
}

int entropy_eval(Packet* last) {
  unordered_map<int, Node_agg0_leaf>::iterator it_agg0_x;
  Node_agg0_leaf *node_agg0_leaf;

  it_agg0_x = node_agg0_x->state_map.find(x);
  if (it_agg0_x == node_agg0_x->state_map.end()) { 
    node_agg0_leaf = &node_agg0_x->default_state;
  } else {
    node_agg0_leaf = &(it_agg0_x->second);

  }
  int ret_agg0 = node_agg0_leaf->sum_agg0;
  return ret_agg0;
}

