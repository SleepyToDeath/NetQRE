all:
	g++-8 -o netqre_client test_client.cpp -lrpc -lpthread -O2 --std=c++14
	g++-8 -o netqre_server server.cpp -lrpc -lpthread -O2 --std=c++14

kill:
	killall -9 netqre_server
