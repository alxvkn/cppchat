CXX = clang++
CXXFLAGS = --std=c++20

DEBUG_FLAGS = -g -DDEBUG=1

all: server client

debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: server client

server: server.o Socket.o
	$(CXX) $^ -o $@

client: client.o Socket.o
	$(CXX) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^

clean:
	rm *.o client server

.PHONY: clean
