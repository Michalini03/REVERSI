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
    
    return 0;
}