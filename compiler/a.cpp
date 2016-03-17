// leaf level
struct Node_2{
int state_0x1788ad0 = 0x178b6e0;
};
// y
struct Node_1 {
unordered_map<int, Node_2> state_map;
Node_2 default_state;
int sum_0x1788c40 = 0;
};
// 
struct Node_0 {
unordered_map<int, Node_1> state_map;
Node_1 default_state;
};
Node_0 state;

int ret_sum_0x1788c40 = 0;

void check_state(,y) {
Node_1& state_1;
if ( in state.state_map) {
state_1 = state.state_map[];
} else {
state_1 = state.default_state;
}
ret_sum_0x1788c40 = state_1.sum_0x1788c40;
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
if (state_2.state_0x1788ad0 == 0x178b6e0) {
state_2.state_0x1788ad0 = 0x178bbc0;
state_1.sum_0x1788c40 += 1 - 0;
}
if (state_2.state_0x1788ad0 == 0x178b960) {
state_2.state_0x1788ad0 = 0x178bbc0;
state_1.sum_0x1788c40 += 1 - 0;
}
return true;
}

int f(ip x, Packet* last) {
  return ret_sum_0x1788c40;
}

