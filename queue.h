#pragma once
#include "player.h"
#include <vector>
#include <ctime>

struct QueueEntry {
    Player player;
    time_t start_time;

    double getWaitTime() const;
};

class MatchmakingQueue {
private:
    std::vector<QueueEntry> queue;

public:
    void enqueue(Player p);
    const std::vector<QueueEntry>& getQueue() const;
};