#pragma once

#include "player.h"
#include <array>
#include <vector>
#include <cstddef>

namespace balance {

struct TeamSplit {
    std::vector<Player> teamA;
    std::vector<Player> teamB;

    double avg_mmr_teamA = 0.0;
    double avg_mmr_teamB = 0.0;
    double mmr_diff = 0.0;
};

class BruteForceBalancer {
public:
    static TeamSplit findBestSplit(const std::array<Player, 10>& players);

private:
    BruteForceBalancer() = delete;
};

}
