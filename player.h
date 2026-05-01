#ifndef PLAYER_H
#define PLAYER_H

#include <string>
#include <ctime>

struct Player {
    int id;
    std::string name;
    int mmr;                   
    std::string preferredRole; 
    time_t joinTime;           
};

double getWaitSeconds(const Player& p);

void printPlayer(const Player& p);

#endif
