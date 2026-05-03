#pragma once
#include <vector>
#include <string>
#include <chrono>
#include <utility>
#include "player.h"   // ✅ Dùng Player từ player.h, xóa struct Player trùng lặp

struct QueueEntry {
    Player player;
    std::chrono::steady_clock::time_point queue_start;
    std::string party_id;
    double wait_time() const;
    std::pair<double, double> mmr_range() const;
};

class Matchmaker {
public:
    void enqueue(const Player& p, std::string party_id = "");
    std::vector<Player> find_match();
private:
    std::vector<QueueEntry> queue;
    const double MMR_INITIAL_RANGE = 150.0;
    const double MMR_EXPAND_PER_SEC = 50.0 / 30.0;
    const double MMR_MAX_RANGE = 600.0;
};