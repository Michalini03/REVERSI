#pragma once
#include <string>

struct Player {
    int socket;
    std::string username;
    int id;

    // Constructor for easy creation
    Player(int s, int id) : socket(s), id(id) {}

    void appendName(const std::string& name) {
        username = name;
    }
};