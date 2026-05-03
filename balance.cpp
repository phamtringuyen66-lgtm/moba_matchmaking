// balance.cpp
#include "balance.h"
#include <cmath> // std::fabs

namespace balance
{

    constexpr double BALANCE_THRESHOLD = 30.0;

    TeamSplit BruteForceBalancer::findBestSplit(const std::array<Player, 10> &players)
    {
        TeamSplit bestSplit;
        bestSplit.mmr_diff = 1e9; // Khởi tạo với giá trị rất lớn

        // Duyệt toàn bộ tổ hợp C(10,5) = 252
        for (int mask = 0; mask < (1 << 10); ++mask)
        {
            if (__popcnt(mask) == 5)
            {
                double sumA = 0.0, sumB = 0.0;
                std::vector<Player> teamA, teamB;

                for (int i = 0; i < 10; ++i)
                {
                    if (mask & (1 << i))
                    {
                        teamA.push_back(players[i]);
                        sumA += players[i].mmr;
                    }
                    else
                    {
                        teamB.push_back(players[i]);
                        sumB += players[i].mmr;
                    }
                }

                double avgA = sumA / 5.0;
                double avgB = sumB / 5.0;
                double diff = std::fabs(avgA - avgB);

                if (diff < bestSplit.mmr_diff)
                {
                    bestSplit.teamA = teamA;
                    bestSplit.teamB = teamB;
                    bestSplit.avg_mmr_teamA = avgA;
                    bestSplit.avg_mmr_teamB = avgB;
                    bestSplit.mmr_diff = diff;
                }
            }
        }

        // Đặt trạng thái dựa trên ngưỡng BALANCE_THRESHOLD
        bestSplit.status = (bestSplit.mmr_diff > BALANCE_THRESHOLD) ? BalanceStatus::UNBALANCED : BalanceStatus::OK;

        return bestSplit;
    }

}