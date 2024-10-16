CXX = clang++
CXXFLAGS = --std=c++17

all: server client

server: server.o
	$(CXX) $^ -o $@

client: client.o
	$(CXX) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^

clean:
	rm *.o client server
