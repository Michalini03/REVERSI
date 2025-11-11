#include "./include/server.h" // Include the server header
#include <iostream>

int main() {
    try {
        // Call the function to start the server
        startServer();
    } catch (const std::exception& e) {
        std::cerr << "A critical error occurred: " << e.what() << std::endl;
        return 1;
    }
    
    // The program will only reach this point if startServer() exits,
    // which it isn't designed to do in its current infinite loop.
    return 0;
}