#include "matchmaker.h"
#include <algorithm>
#include <cmath>

// Tính thời gian chờ thực tế tính bằng giây
double QueueEntry::wait_time() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now - queue_start).count();
}

// Mở rộng dải MMR chấp nhận theo thời gian chờ
std::pair<double, double> QueueEntry::mmr_range() const {
    double wait = wait_time();
    double expand = std::min(150.0 + wait * (50.0 / 30.0), 600.0);
    return { player.mmr - expand, player.mmr + expand };
}

void Matchmaker::enqueue(const Player& p, std::string party_id) {
    queue.push_back({ p, std::chrono::steady_clock::now(), party_id });
}

std::vector<Player> Matchmaker::find_match() {
    if (queue.size() < 10) return {};

    // Sắp xếp theo MMR tăng dần
    std::sort(queue.begin(), queue.end(), [](const QueueEntry& a, const QueueEntry& b) {
        return a.player.mmr < b.player.mmr;
        });

    for (size_t i = 0; i < queue.size(); ++i) {
        auto& anchor = queue[i];
        auto range = anchor.mmr_range();
        std::vector<size_t> indices;
        indices.push_back(i);

        for (size_t j = 0; j < queue.size(); ++j) {
            if (i == j) continue;
            double target_mmr = queue[j].player.mmr;
            if (target_mmr >= range.first && target_mmr <= range.second) {
                indices.push_back(j);
            }
            if (indices.size() == 10) break;
        }

        if (indices.size() == 10) {
            std::vector<Player> match_group;
            std::sort(indices.rbegin(), indices.rend());
            for (size_t idx : indices) {
                match_group.push_back(queue[idx].player);
                queue.erase(queue.begin() + idx);
            }
            return match_group;
        }
    }

    return {};
}