CC=g++
CFLAGS=-std=c++17 -Wall -Wextra
TARGET=propulsion_server
SRC_DIR=c++
OBJECTS=$(SRC_DIR)/main.o $(SRC_DIR)/propulsion_server.o
ZMQ_OBJS=zmq_$(SRC_DIR)/main.o zmq_$(SRC_DIR)/propulsion_server.o
ZMQ_TARGET=propulsion_server_zmq
LDFLAGS=-lzmq

all: $(TARGET) $(ZMQ_TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(ZMQ_TARGET): $(ZMQ_OBJS)
	$(CC) $(CFLAGS) -o $(ZMQ_TARGET) $(ZMQ_OBJS) $(LDFLAGS)

zmq_$(SRC_DIR)/%.o: zmq_$(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS) $(ZMQ_TARGET) $(ZMQ_OBJS)