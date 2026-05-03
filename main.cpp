#include <iostream>
#include <iomanip>
#include <array>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include "player.h"
#include "queue.h"
#include "balance.h"
#include "hungarian.h"
#include "elo.h"

using namespace std;

static const string ROLE_NAMES[5] = { "Top", "Jungle", "Mid", "AD", "Support" };

hungarian::CostMatrix buildCostMatrix(const vector<Player>& team) {
    hungarian::CostMatrix matrix;
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            if (team[i].preferredRole == ROLE_NAMES[j]) {
                matrix[i][j] = 0;
            }
            else {
                int altRole = (j + 1) % 5;
                if (team[i].preferredRole == ROLE_NAMES[altRole]) {
                    matrix[i][j] = 1;
                }
                else {
                    matrix[i][j] = 10;
                }
            }
        }
    }
    return matrix;
}

void printDivider(char c = '=', int n = 60) {
    cout << string(n, c) << "\n";
}

// Chon MVP ngau nhien tu doi
int chonMVP(const vector<Player>& team) {
    return rand() % team.size();
}

void runMatch(int matchNo, MatchQueue& queue) {
    printDivider();
    cout << "  TRAN DAU " << matchNo << "\n";
    printDivider();

    // Buoc 1: Lay 10 nguoi choi
    cout << "\n[Buoc 1] Lay 10 nguoi choi tu hang cho...\n";
    array<Player, 10> players;
    int got = queue.getTopPlayers(players.data(), 10);
    if (got < 10) {
        cout << "   -> Khong du nguoi choi! Chi co " << got << " nguoi.\n";
        return;
    }
    for (int i = 0; i < 10; ++i) {
        cout << "   " << setw(2) << (i + 1) << ". "
            << setw(12) << left << players[i].name
            << " MMR: " << setw(5) << players[i].mmr
            << " | Role: " << players[i].preferredRole << "\n";
    }

    // Buoc 2: Team Balancing C(10,5)
    cout << "\n[Buoc 2] Team Balancing - Duyet C(10,5) = 252 phuong an...\n";
    balance::BruteForceBalancer balancer;
    balance::TeamSplit split = balancer.findBestSplit(players);

    cout << "   -> Team A (avg MMR: " << fixed << setprecision(2)
        << split.avg_mmr_teamA << "): ";
    for (auto& p : split.teamA) cout << p.name << " ";
    cout << "\n";
    cout << "   -> Team B (avg MMR: " << split.avg_mmr_teamB << "): ";
    for (auto& p : split.teamB) cout << p.name << " ";
    cout << "\n";
    cout << "   -> Do lech MMR: " << split.mmr_diff
        << (split.mmr_diff > 30 ? " [UNBALANCED]" : " [OK - Can bang]") << "\n";

    // Buoc 3: Hungarian Algorithm
    cout << "\n[Buoc 3] Phan cong vai tro - Hungarian Algorithm O(n^3)...\n";

    cout << "   >> Team A:\n";
    hungarian::CostMatrix matA = buildCostMatrix(split.teamA);
    hungarian::HungarianSolver solverA;
    auto assignA = solverA.solve(matA);
    if (assignA) {
        cout << "      Tong chi phi toi uu: " << assignA->total_cost << "\n";
        for (int i = 0; i < 5; ++i) {
            string role = ROLE_NAMES[assignA->roles[i]];
            bool isMain = (split.teamA[i].preferredRole == role);
            cout << "      " << setw(12) << left << split.teamA[i].name
                << " -> " << setw(9) << role
                << (isMain ? "(role chinh)" : "(auto-fill)") << "\n";
        }
    }

    cout << "   >> Team B:\n";
    hungarian::CostMatrix matB = buildCostMatrix(split.teamB);
    hungarian::HungarianSolver solverB;
    auto assignB = solverB.solve(matB);
    if (assignB) {
        cout << "      Tong chi phi toi uu: " << assignB->total_cost << "\n";
        for (int i = 0; i < 5; ++i) {
            string role = ROLE_NAMES[assignB->roles[i]];
            bool isMain = (split.teamB[i].preferredRole == role);
            cout << "      " << setw(12) << left << split.teamB[i].name
                << " -> " << setw(9) << role
                << (isMain ? "(role chinh)" : "(auto-fill)") << "\n";
        }
    }

    // Buoc 4: Cap nhat Glicko-2 + MVP
    cout << "\n[Buoc 4] Cap nhat MMR Glicko-2 (Team A thang)...\n";

    // Luu MMR cu truoc khi update
    vector<double> oldMmrA, oldMmrB;
    for (auto& p : split.teamA) oldMmrA.push_back(p.mmr);
    for (auto& p : split.teamB) oldMmrB.push_back(p.mmr);

    // Chon MVP ngau nhien moi doi
    int mvpA = chonMVP(split.teamA);
    int mvpB = chonMVP(split.teamB);

    cout << "   >> MVP Team A (THANG): " << split.teamA[mvpA].name << " *** MVP ***\n";
    cout << "   >> MVP Team B (THUA) : " << split.teamB[mvpB].name << " *** MVP ***\n\n";

    // Cap nhat MMR theo Glicko-2
    Glicko2::update_match(split.teamA, split.teamB, true);

    // Bonus MVP:
    // - Doi thang: MVP duoc cong them 15% diem thang
    // - Doi thua : MVP chi bi tru 60% diem thua (giam 40% hinh phat)
    constexpr double MVP_WIN_BONUS = 0.15; // cong them 15%
    constexpr double MVP_LOSE_SAVE = 0.40; // giam 40% hinh phat

    cout << "   >> Team A (THANG):\n";
    for (int i = 0; i < 5; ++i) {
        double delta = split.teamA[i].mmr - oldMmrA[i];
        if (i == mvpA && delta > 0) {
            double bonus = delta * MVP_WIN_BONUS;
            split.teamA[i].mmr += bonus;
            delta += bonus;
            cout << "      " << setw(12) << left << split.teamA[i].name
                << " MMR: " << fixed << setprecision(0) << oldMmrA[i]
                << " -> " << split.teamA[i].mmr
                << "  (+" << delta << " diem) *** MVP BONUS +15% ***\n";
        }
        else {
            cout << "      " << setw(12) << left << split.teamA[i].name
                << " MMR: " << fixed << setprecision(0) << oldMmrA[i]
                << " -> " << split.teamA[i].mmr
                << "  (+" << delta << " diem)\n";
        }
    }

    cout << "   >> Team B (THUA):\n";
    for (int i = 0; i < 5; ++i) {
        double delta = oldMmrB[i] - split.teamB[i].mmr; // delta duong = mat diem
        if (i == mvpB && delta > 0) {
            double saved = delta * MVP_LOSE_SAVE;
            split.teamB[i].mmr += saved; // hoan lai 40% diem bi tru
            delta -= saved;
            cout << "      " << setw(12) << left << split.teamB[i].name
                << " MMR: " << fixed << setprecision(0) << oldMmrB[i]
                << " -> " << split.teamB[i].mmr
                << "  (-" << delta << " diem) *** MVP -40% HINH PHAT ***\n";
        }
        else {
            cout << "      " << setw(12) << left << split.teamB[i].name
                << " MMR: " << fixed << setprecision(0) << oldMmrB[i]
                << " -> " << split.teamB[i].mmr
                << "  (-" << delta << " diem)\n";
        }
    }
    cout << "\n";
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    cout << "========== HE THONG MATCHMAKING MOBA - NHOM 10 ==========\n\n";

    MatchQueue matchQueue;

    struct MockData { string name; int mmr; string role; };

    // =============================================
    // TRAN 1
    // =============================================
    cout << "[Khoi tao] Tran 1 - 10 nguoi choi:\n";

    const MockData match1[10] = {
        {"Bang",     1600, "Jungle" },
        {"Fish",     1550, "Mid"    },
        {"Shin",     1500, "ADC"    },
        {"Nolan",    1450, "Top"    },
        {"Khoa",     1400, "Support"},
        {"Gray",     1580, "Jungle" },
        {"Maris",    1530, "Mid"    },
        {"BeTroc",   1480, "ADC"    },
        {"HuyHoang", 1430, "Top"    },
        {"Yutan",    1480, "Support"},
    };

    for (int i = 0; i < 10; ++i) {
        Player p;
        p.id = i + 1;
        p.name = match1[i].name;
        p.mmr = match1[i].mmr;
        p.mmr_deviation = 200.0;
        p.volatility = 0.06;
        p.preferredRole = match1[i].role;
        p.joinTime = time(nullptr);
        matchQueue.addPlayer(p);
        cout << "   [+] " << setw(10) << left << p.name
            << " MMR: " << setw(5) << p.mmr
            << " | Role: " << p.preferredRole << "\n";
    }

    cout << "\n[MMR Range Expansion]\n";
    cout << "   t=0s   -> range = " << matchQueue.getMmrRange(0) << " diem\n";
    cout << "   t=30s  -> range = " << matchQueue.getMmrRange(30) << " diem\n";
    cout << "   t=60s  -> range = " << matchQueue.getMmrRange(60) << " diem\n";
    cout << "   t=300s -> range = " << matchQueue.getMmrRange(300) << " diem (MAX)\n\n";

    runMatch(1, matchQueue);

    // =============================================
    // TRAN 2
    // =============================================
    cout << "[Khoi tao] Tran 2 - 10 nguoi choi (A-Z):\n";

    const MockData match2[10] = {
        {"Alice",  1720, "Top"    },
        {"Bruno",  1680, "Jungle" },
        {"Clara",  1640, "Mid"    },
        {"Derek",  1600, "ADC"    },
        {"Elena",  1560, "Support"},
        {"Felix",  1740, "Top"    },
        {"Gloria", 1700, "Jungle" },
        {"Hugo",   1660, "Mid"    },
        {"Iris",   1620, "ADC"    },
        {"Jonas",  1580, "Support"},
    };

    for (int i = 0; i < 10; ++i) {
        Player p;
        p.id = 11 + i;
        p.name = match2[i].name;
        p.mmr = match2[i].mmr;
        p.mmr_deviation = 180.0;
        p.volatility = 0.06;
        p.preferredRole = match2[i].role;
        p.joinTime = time(nullptr);
        matchQueue.addPlayer(p);
        cout << "   [+] " << setw(10) << left << p.name
            << " MMR: " << setw(5) << p.mmr
            << " | Role: " << p.preferredRole << "\n";
    }

    cout << "\n";
    runMatch(2, matchQueue);

    printDivider('=', 60);
    cout << "  MO PHONG HOAN TAT - 2 TRAN DA CHAY THANH CONG\n";
    cout << "  Thuat toan da su dung:\n";
    cout << "  [1] MMR Range Expansion (tuyen tinh)\n";
    cout << "  [2] Team Balancing Brute-force C(10,5) = 252\n";
    cout << "  [3] Hungarian Algorithm O(n^3)\n";
    cout << "  [4] Glicko-2 MMR Update + He thong MVP\n";
    printDivider('=', 60);

    return 0;
}