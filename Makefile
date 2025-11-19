# Variables
SERVER_SRCS = server/main.cpp server/src/lobby.cpp server/src/server.cpp 
SERVER_BIN = server/server

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
	@echo "Running client..."
	python3 client/main.py

# Run virtual environment for client
run-venv:
	@echo "Activating virtual environment and running client..."

# Clean build files
clean:
	@echo "Cleaning up..."
	rm -f $(SERVER_BIN)
