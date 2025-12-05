# Variables
SERVER_SRCS = server/main.cpp server/src/lobby.cpp server/src/server.cpp server/src/gameLogic.cpp
SERVER_BIN = server/server
VENV_ACTIVATE = .venv/bin/activate

# Default target (runs when you just type `make`)
all: $(SERVER_BIN)

# Compile the server
$(SERVER_BIN): $(SERVER_SRCS)
	@echo "Compiling server..."
	g++ -std=c++17 -O2 -o $(SERVER_BIN) $(SERVER_SRCS)

# Run the server
run-server: $(SERVER_BIN)
	@echo "Running server..."
	./$(SERVER_BIN)

# Run the Python client
run-client:
ifeq (,$(wildcard .venv))
	@echo ".venv does not exist. Creating virtual environment..."
	python3 -m venv .venv
	.venv/bin/pip install -r requirements.txt
endif
	@echo "Running client..."
	.venv/bin/python client/main.py

# Run virtual environment for client
run-venv:
	@echo "Activating virtual environment and running client..."

# Clean build files
clean:
	@echo "Cleaning up..."
	rm -f $(SERVER_BIN)
