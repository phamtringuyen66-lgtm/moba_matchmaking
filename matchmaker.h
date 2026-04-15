#pragma once

#include "queue.h"
#include "balance.h"
#include "hungarian.h"
#include "player.h"

#include <array>
#include <vector>
#include <optional>
#include <chrono>
#include <cstddef>

namespace matchmaker {

using Seconds = std::chrono::seconds;

struct MatchResult {
    std::vector<Player> teamA;
    std::vector<Player> teamB;
    hungarian::Assignment assignmentA;
    hungarian::Assignment assignmentB;
    double combined_cost = 0.0;
    std::chrono::system_clock::time_point created_at;
};

class Matchmaker {
public:
    Matchmaker() = default;
    ~Matchmaker() = default;

    void tick();
    std::optional<std::array<Player, 10>> find_candidates(const Player& anchor, const Seconds& wait_seconds) const;
    MatchResult assign_roles_and_build(const balance::TeamSplit& split) const;
    void enqueue_player(const Player& p);
    const MatchmakingQueue& get_queue() const;

private:
    static double mmr_range_for_wait(const Seconds& wait_seconds);
    MatchmakingQueue queue_;
};

} // namespace matchmaker
