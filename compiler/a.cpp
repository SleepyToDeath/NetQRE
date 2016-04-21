// leaf
struct Node_agg0_leaf{
};
// x
struct Node_agg0_x{
int sum_agg0;
int state_re0;
unordered_map<int, Node_agg0_leaf> stateMap;
Node_agg0_leaf default_state;
};
// x_f_agg0
struct Node_agg0_x_f_agg0{
unordered_map<int, Node_agg0_x> stateMap;
Node_agg0_x default_state;
};
// leaf
struct Node_agg1_leaf{
};
// x
struct Node_agg1_x{
int sum_agg1;
int state_re1;
unordered_map<int, Node_agg1_leaf> stateMap;
Node_agg1_leaf default_state;
};
// x_f_agg1
struct Node_agg1_x_f_agg1{
unordered_map<int, Node_agg1_x> stateMap;
Node_agg1_x default_state;
};
