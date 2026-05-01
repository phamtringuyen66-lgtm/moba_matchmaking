#ifndef QUEUE_H
#define QUEUE_H

#include <queue>
#include "player.h"

struct CompareWait {
    bool operator()(const Player& a, const Player& b) const {
        return a.joinTime > b.joinTime;
    }
};

class MatchQueue {
private:
    std::priority_queue<Player, std::vector<Player>, CompareWait> waitingList;

public:
    void addPlayer(const Player& p);

    Player popFront();

    int getTopPlayers(Player* result, int n) const;

    bool isEmpty() const;

    int size() const;

    void printQueue() const;
};

#endif
