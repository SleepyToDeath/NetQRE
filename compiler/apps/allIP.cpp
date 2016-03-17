// leaf level
struct Node_1{
    int state_0x997410 = 0x999c00;
};
// x
struct Node_0 {
    map<int, Node_1> state_map;
    Node_1 default_state;
    int sum_0x997580 = 0;
};
Node_0 state;

int ret_sum_0x997580 = 0;

void check_state(x) {
    ret_sum_0x997580 = state.sum_0x997580;
    Node_1& state_1;
    if (x in state.state_map) {
	state_1 = state.state_map[x];
    } else {
	state_1 = state.default_state;
    }
}

bool allip_update(Packet *last) {
    if (srcip not in state.state_map) { 
	state.state_map[srcip] = state.default_state;
    }
    Node_1& state_1 = state.state_map[srcip];
    if (state_1.state_0x997410 == 0x999c00) {
	state_1.state_0x997410 = 0x99a180;
	state.sum_0x997580 += 1 - 0;
    }
    return true;
}

int allip(Packet* last) {
    return ret_sum_0x997580;
}

