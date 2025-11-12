#include "../include/server.h"      // Include the header file we will create
#include <iostream>      // For standard I/O operations (cout, cerr)
#include <string>        // For using std::string
#include <cstring>       // For C-style string functions (memset)
#include <sys/socket.h>  // For socket functions (socket, bind, listen, accept)
#include <netinet/in.h>  // For sockaddr_in structure and INADDR_ANY
#include <unistd.h>      // For close() function

// Define a port number for the server
#define PORT 9999

void startServer() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0}; // Buffer to store received data

    // --- 1. Create a socket ---
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // --- 2. Bind the socket to an address and port ---
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // --- 3. Listen for incoming connections ---
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on port " << PORT << "..." << std::endl;

    // --- 4. Start the infinite loop to accept clients ---
    while(true) {
        std::cout << "\nWaiting for a new connection..." << std::endl;

        // --- 4a. Accept a new connection ---
        // This is a blocking call. The program will wait here until a client connects.
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            // Don't exit, just log the error and continue listening
            continue; 
        }

        std::cout << "Connection accepted!" << std::endl;

        // --- 4b. Receive data from the client ---
        // We'll use a loop here to keep handling messages until the client disconnects
        while(true) {
            // Clear the buffer
            memset(buffer, 0, 1024);

            // Read data from the client into the buffer
            int valread = read(new_socket, buffer, 1024);
            
            if (valread <= 0) {
                // If read returns 0 or -1, the client has disconnected or an error occurred
                std::cout << "Client disconnected." << std::endl;
                break; // Exit the inner loop
            } else {
                std::cout << "Client said: " << buffer << std::endl;
            }

            // --- 4c. Send a response to the client ---
            const char *hello = "Message received!";
            send(new_socket, hello, strlen(hello), 0);
            std::cout << "Response sent to client." << std::endl;
        }

        // --- 4d. Close the client-specific socket ---
        // We're done with this client, close their socket and loop back to accept()
        close(new_socket);
    }

    // --- 5. Close the main listening socket ---
    close(server_fd);
}