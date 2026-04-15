#pragma once

#include <utility>

namespace elo {

enum class Outcome {
    TeamA_Win,
    TeamB_Win,
    Draw
};

class Elo {
public:
    Elo() = delete;
    static std::pair<double, double> calculate_delta(double mmr_team_a, double mmr_team_b, Outcome outcome, double K = 32.0);
};

} // namespace elo
