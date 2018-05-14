all:client server
.PHONY:all


client: client.cpp 
	g++ -o client client.cpp -lnsl -lresolv
server: server.cpp
	g++ -o server server.cpp -lnsl -lresolv
