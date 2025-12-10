#pragma once
#include <string>

struct Player {
    int socket;
    int tolerance;
    std::string username;

    // Constructor for easy creation
    Player(int s) : socket(s), tolerance(0) {}

    void appendName(const std::string& name) {
        username = name;
    }
};