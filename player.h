#pragma once
#include <string>
#include <vector>

struct Player {
    std::string id;
    std::string name;
    double mmr = 1500.0;
    double mmr_deviation = 350.0;
    double volatility = 0.06;
    std::vector<std::string> preferred_roles;
};