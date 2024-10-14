CXX = clang++

all: server client

server: server.o
	$(CXX) $^ -o $@

client: client.o
	$(CXX) $^ -o $@

%.o: %.cpp
	$(CXX) -c $^

clean: rm *.o client server
