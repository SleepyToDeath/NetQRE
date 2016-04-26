// leaf
struct Node_agg0_leaf{
};
// 
struct Node_agg0_{
int sum_agg0;
int state_re0;
unordered_map<int, Node_agg0_leaf> stateMap;
Node_agg0_leaf default_state;
};
// y
struct Node_agg0_y{
unordered_map<int, Node_agg0_> stateMap;
Node_agg0_ default_state;
};
// x
struct Node_agg0_x{
unordered_map<int, Node_agg0_y> stateMap;
Node_agg0_y default_state;
};
