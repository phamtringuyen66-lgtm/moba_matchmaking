#pragma once

#include "player.h"
#include <vector>
#include <utility>

// K?t qu? tr?n (dùng trong Elo/Glicko2)
enum class MatchOutcome { WIN = 1, LOSS = 0, DRAW = 2 };
inline double outcome_score(MatchOutcome o);

// ELO namespace
namespace Elo {
    constexpr double K_FACTOR = 32.0;
    double expected_score(double mmr_a, double mmr_b);
    void update(Player& player, double opp_mmr, MatchOutcome outcome);
    void update_match(std::vector<Player>& team_a, std::vector<Player>& team_b, bool a_wins);
}

// Glicko-2 namespace
namespace Glicko2 {
    constexpr double TAU = 0.5;
    constexpr double EPSILON = 0.000001;
    constexpr double SCALE = 173.7178;

    struct OpponentRecord { double mmr; double rd; double score; };

    double g_phi(double phi);
    double E_func(double mu, double mu_j, double phi_j);
    void update_single(Player& player, const std::vector<OpponentRecord>& opponents);
    void update_match(std::vector<Player>& team_a, std::vector<Player>& team_b, bool a_wins);
}
