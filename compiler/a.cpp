// leaf level
struct Node_2{
int sum_0x716bc0 = 0;
int state_0x716a50 = 0x7197d0;
};
// yy
struct Node_1 {
unordered_map<int, Node_2> state_map;
Node_2 default_state;
};
// x
struct Node_0 {
unordered_map<int, Node_1> state_map;
Node_1 default_state;
};
Node_0 state;

int ret_sum_0x716bc0 = 0;

void check_state(x,yy) {
Node_1& state_1;
if (x in state.state_map) {
state_1 = state.state_map[x];
} else {
state_1 = state.default_state;
}
Node_2& state_2;
if (yy in state_1.state_map) {
state_2 = state_1.state_map[yy];
} else {
state_2 = state_1.default_state;
}
ret_sum_0x716bc0 = state_2.sum_0x716bc0;
}

bool f_update(Packet *last) {
auto it0 = state.state_map.find(src);
if (it0 == state.state_map.end()) { 
it0 = state.state_map.insert({src, state.default_state}).first;
}
Node_1& state_1 = it0.second;
auto it1 = state_1.state_map.find(dst+1);
if (it1 == state_1.state_map.end()) { 
it1 = state_1.state_map.insert({dst+1, state_1.default_state}).first;
}
Node_2& state_2 = it1.second;
if (state_2.state_0x716a50 == 0x7197d0) {
state_2.state_0x716a50 = 0x719cb0;
state_2.sum_0x716bc0 += 1 - 0;
}
if (state_2.state_0x716a50 == 0x719a50) {
state_2.state_0x716a50 = 0x719cb0;
state_2.sum_0x716bc0 += 1 - 0;
}
return true;
}

int f(ip x, Packet* last) {
  return ret_sum_0x716bc0;
}

