#include "matchmaker.h"
#include <algorithm>
#include <cmath>

// tính time chờ thực tế tính bằng giây 
double QueueEntry::wait_time() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double>(now - queue_start).count();
}

// mở rộng dải MMR chấp nhận theo thời gian chờ 
std::pair<double, double> QueueEntry::mmr_range() const {
    double wait = wait_time();
    // range(t) = min(150 + t * 1.67, 600) 
    double expand = std::min(150.0 + wait * (50.0 / 30.0), 600.0);
    return { player.mmr - expand, player.mmr + expand };
}

void Matchmaker::enqueue(const Player& p, std::string party_id) {
    queue.push_back({ p, std::chrono::steady_clock::now(), party_id });
}

std::vector<Player> Matchmaker::find_match() {
    // Chỉ thực hiện khi có đủ tối thiểu 10 người 
    if (queue.size() < 10) return {};

    // sắp xếp theo MMR tăng dần để tối ưu hóa việc quét candidates 
    std::sort(queue.begin(), queue.end(), [](const QueueEntry& a, const QueueEntry& b) {
        return a.player.mmr < b.player.mmr;
        });

    // thuật toán tìm candidates 
    for (size_t i = 0; i < queue.size(); ++i) {
        auto& anchor = queue[i];
        auto range = anchor.mmr_range();

        std::vector<size_t> indices;
        indices.push_back(i);

        for (size_t j = 0; j < queue.size(); ++j) {
            if (i == j) continue;

            double target_mmr = queue[j].player.mmr;
            // Kiểm tra xem người chơi j có nằm trong dải MMR của anchor không 
            if (target_mmr >= range.first && target_mmr <= range.second) {
                indices.push_back(j);
            }

            if (indices.size() == 10) break;
        }

        // Nếu tìm đủ 10 người tương thích 
        if (indices.size() == 10) {
            std::vector<Player> match_group;

            // Sắp xếp chỉ số giảm dần để xóa khỏi vector an toàn
            std::sort(indices.rbegin(), indices.rend());

            for (size_t idx : indices) {
                match_group.push_back(queue[idx].player);
                queue.erase(queue.begin() + idx); // Xóa khỏi hàng chờ 
            }
            return match_group;
        }
    }

    return {};
}