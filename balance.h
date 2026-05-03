#pragma once

#include "player.h"
#include <array>
#include <vector>
#include <cstddef>

namespace balance {

enum class BalanceStatus {
    OK,
    UNBALANCED
};

struct TeamSplit {
    std::vector<Player> teamA;
    std::vector<Player> teamB;

    double avg_mmr_teamA = 0.0;
    double avg_mmr_teamB = 0.0;
    double mmr_diff = 0.0;

    BalanceStatus status = BalanceStatus::OK;
};

class BruteForceBalancer {
public:
    BruteForceBalancer() = default;
    static TeamSplit findBestSplit(const std::array<Player, 10>& players);

    BruteForceBalancer(const BruteForceBalancer&) = default;
    BruteForceBalancer(BruteForceBalancer&&) noexcept = default;
    BruteForceBalancer& operator=(const BruteForceBalancer&) = default;
    BruteForceBalancer& operator=(BruteForceBalancer&&) noexcept = default;
};

} // namespace balance
