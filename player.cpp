#include "player.h"
#include <iostream>

double getWaitSeconds(const Player& p) {
    time_t now = time(nullptr);
    return difftime(now, p.joinTime);
}

void printPlayer(const Player& p) {
    std::cout << "[Player]"
        << " ID: " << p.id
        << " | Name: " << p.name
        << " | MMR: " << p.mmr
        << " | Role: " << p.preferredRole
        << " | Wait: " << getWaitSeconds(p) << "s"
        << std::endl;
}