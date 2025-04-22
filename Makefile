CC=g++
CFLAGS=-std=c++17 -Wall -Wextra
TARGET=propulsion_server
SRC_DIR=src
OBJECTS=$(SRC_DIR)/main.o $(SRC_DIR)/propulsion_server.o

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)