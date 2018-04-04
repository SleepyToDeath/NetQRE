Packet pkt;

int main() {
    while (true) {
	cout << "src = ";
	cin >> pkt.src;
	
	cout << "dst = ";
	cin >> pkt.dst;

	smain_update(&pkt);
	cout << "result is: \t" << smain_eval(&pkt) << endl;
    }
    return 1;
}
