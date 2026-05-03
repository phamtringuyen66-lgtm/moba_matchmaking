#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <ctime>

struct Player {
    int id = 0;
    std::string name = "";
    int mmr = 1500;
    double mmr_deviation = 350.0;
    double volatility = 0.06;
    std::string preferredRole = "";
    time_t joinTime = time(nullptr);
};

double getWaitSeconds(const Player& p);
void printPlayer(const Player& p);

#endif