#pragma once
#include <string>
#include <vector>

struct Player {
    std::string id;
    std::string name;
    double mmr;
    std::vector<std::string> preferred_roles;
};