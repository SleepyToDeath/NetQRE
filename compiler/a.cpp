// leaf level
struct Node_2{
int state_0x1a79a50 = 0x1a7c690;
};
// y
struct Node_1 {
unordered_map<int, Node_2> state_map;
Node_2 default_state;
int sum_0x1a79bc0 = 0;
};
// x-1
struct Node_0 {
unordered_map<int, Node_1> state_map;
Node_1 default_state;
};
Node_0 state;

int ret_sum_0x1a79bc0 = 0;

void check_state(x-1,y) {
Node_1& state_1;
if (x-1 in state.state_map) {
state_1 = state.state_map[x-1];
} else {
state_1 = state.default_state;
}
ret_sum_0x1a79bc0 = state_1.sum_0x1a79bc0;
Node_2& state_2;
if (y in state_1.state_map) {
state_2 = state_1.state_map[y];
} else {
state_2 = state_1.default_state;
}
}

bool f_update(Packet *last) {
auto it0 = state.state_map.find(src);
if (it0 == state.state_map.end()) { 
it0 = state.state_map.insert({src, state.default_state}).first;
}
Node_1& state_1 = it0.second;
auto it1 = state_1.state_map.find(dst);
if (it1 == state_1.state_map.end()) { 
it1 = state_1.state_map.insert({dst, state_1.default_state}).first;
}
Node_2& state_2 = it1.second;
if (state_2.state_0x1a79a50 == 0x1a7c690) {
state_2.state_0x1a79a50 = 0x1a7cb70;
state_1.sum_0x1a79bc0 += 1 - 0;
}
if (state_2.state_0x1a79a50 == 0x1a7c910) {
state_2.state_0x1a79a50 = 0x1a7cb70;
state_1.sum_0x1a79bc0 += 1 - 0;
}
return true;
}

int f(ip x, Packet* last) {
  return ret_sum_0x1a79bc0;
}

