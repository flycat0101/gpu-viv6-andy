.PHONY: all clean

BIN_DIR=../../bin

all: $(BIN_DIR)/resource

$(BIN_DIR)/resource: main.cpp
	@-mkdir -p $(BIN_DIR)
	@gcc -o $(BIN_DIR)/resource -O -Wall main.cpp -lstdc++

clean:
	@-rm $(BIN_DIR)/resource


