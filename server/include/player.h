#pragma once
#include <string>

struct Player {
    int socket;
    std::string username;

    // Constructor for easy creation
    Player(int s) : socket(s) {}

    void appendName(const std::string& name) {
        username = name;
    }
};