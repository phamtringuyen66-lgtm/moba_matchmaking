#include "elo.h"
#include <cmath>
#include <vector>
#include <numeric>
#include <limits>
#include <algorithm>

namespace {

// Pi h?ng
constexpr double PI = 3.14159265358979323846;

} // namespace

// Hàm ti?n: ánh x? k?t qu? tr?n ??u sang ?i?m (WIN=1.0, LOSS=0.0, DRAW=0.5)
double outcome_score(MatchOutcome o) {
    switch (o) {
        case MatchOutcome::WIN:  return 1.0;
        case MatchOutcome::LOSS: return 0.0;
        case MatchOutcome::DRAW: return 0.5;
        default: return 0.0;
    }
}

namespace Elo {

double expected_score(double mmr_a, double mmr_b) {
    // E = 1 / (1 + 10^{(mmr_b - mmr_a)/400})
    return 1.0 / (1.0 + std::pow(10.0, (mmr_b - mmr_a) / 400.0));
}

void update(Player& player, double opp_mmr, MatchOutcome outcome) {
    // L?y ?i?m th?c t? S
    double S = outcome_score(outcome);
    // Tính k? v?ng E
    double E = expected_score(player.mmr, opp_mmr);
    // C?p nh?t MMR: MMR_new = MMR_old + K * (S - E)
    player.mmr += K_FACTOR * (S - E);
}

void update_match(std::vector<Player>& team_a, std::vector<Player>& team_b, bool a_wins) {
    // Tính MMR trung bình c?a m?i ??i
    auto avg_mmr = [](const std::vector<Player>& team) -> double {
        if (team.empty()) return 0.0;
        double s = std::accumulate(team.begin(), team.end(), 0.0,
            [](double acc, const Player& p){ return acc + p.mmr; });
        return s / static_cast<double>(team.size());
    };

    double avg_b = avg_mmr(team_b);
    double avg_a = avg_mmr(team_a);

    // K?t qu? ??i A so v?i ??i B
    MatchOutcome outcome_a = a_wins ? MatchOutcome::WIN : MatchOutcome::LOSS;
    if (!a_wins) {
        // n?u A thua thì B th?ng; n?u A th?ng thì B thua
        // chúng ta s? x? lý ??i B t??ng ?ng
    }

    // C?p nh?t t?ng ng??i trong team A v?i opp_mmr = avg_b
    for (auto& p : team_a) {
        update(p, avg_b, a_wins ? MatchOutcome::WIN : (a_wins==false ? MatchOutcome::LOSS : MatchOutcome::DRAW));
    }

    // C?p nh?t t?ng ng??i trong team B v?i opp_mmr = avg_a (k?t qu? ??o)
    for (auto& p : team_b) {
        update(p, avg_a, a_wins ? MatchOutcome::LOSS : MatchOutcome::WIN);
    }
}

} // namespace Elo

namespace Glicko2 {

// g(phi) = 1 / sqrt(1 + 3*phi^2 / pi^2)
double g_phi(double phi) {
    return 1.0 / std::sqrt(1.0 + 3.0 * phi * phi / (PI * PI));
}

// E(mu, mu_j, phi_j) = 1 / (1 + exp(-g(phi_j) * (mu - mu_j)))
double E_func(double mu, double mu_j, double phi_j) {
    double g = g_phi(phi_j);
    return 1.0 / (1.0 + std::exp(-g * (mu - mu_j)));
}

// C?p nh?t m?t ng??i ch?i theo danh sách ??i th? (theo Glicko-2)
void update_single(Player& player, const std::vector<OpponentRecord>& opponents) {
    if (opponents.empty()) {
        // Không có ??i th? -> không c?p nh?t
        return;
    }

    // 1) Chuy?n ??i rating và RD sang h? chu?n (mu, phi)
    double mu = (player.mmr - 1500.0) / SCALE;
    double phi = player.mmr_deviation / SCALE;
    double sigma = player.volatility;

    // 2) Tính g(phi_j), E_j cho t?ng ??i th? và các t?ng c?n thi?t
    double sum_g2_E1E = 0.0; // sum g^2 * E * (1 - E)
    double sum_g_s_minus_E = 0.0; // sum g * (s_j - E_j)

    for (const auto& opp : opponents) {
        double mu_j = (opp.mmr - 1500.0) / SCALE;
        double phi_j = opp.rd / SCALE;
        double g = g_phi(phi_j);
        double E = E_func(mu, mu_j, phi_j);
        sum_g2_E1E += (g * g) * E * (1.0 - E);
        sum_g_s_minus_E += g * (opp.score - E);
    }

    // 3) v = 1 / sum_g2_E1E
    double v = 1.0 / sum_g2_E1E;

    // 4) delta = v * sum_g_s_minus_E
    double delta = v * sum_g_s_minus_E;

    // 5) Tìm sigma' (volatility m?i) b?ng thu?t toán tìm nghi?m (theo Glickman)
    double a = std::log(sigma * sigma);

    auto f = [&](double x) {
        double ex = std::exp(x);
        double num = ex * (delta * delta - phi * phi - v - ex);
        double den = 2.0 * std::pow(phi * phi + v + ex, 2.0);
        return num / den - (x - a) / (TAU * TAU);
    };

    // Kh?i t?o A = a
    double A = a;
    double B;
    if (delta * delta > (phi * phi + v)) {
        B = std::log(delta * delta - phi * phi - v);
    } else {
        // Tìm B sao cho f(B) < 0
        double k = 1.0;
        while (f(a - k * TAU) > 0.0) {
            k *= 2.0;
            // b?o ??m không vô h?n
            if (k > 1e9) break;
        }
        B = a - k * TAU;
    }

    double fA = f(A);
    double fB = f(B);

    // Illinois-style regula falsi / secant iteration
    while (std::abs(B - A) > EPSILON) {
        // Secant step
        double C = A + (A - B) * fA / (fB - fA);
        double fC = f(C);

        if (fC * fB < 0.0) {
            A = B;
            fA = fB;
            B = C;
            fB = fC;
        } else {
            // adjust fA for Illinois method
            fA = fA / 2.0;
            B = C;
            fB = fC;
        }
    }

    double a_prime = B;
    double sigma_prime = std::exp(a_prime / 2.0);

    // 6) phi_star = sqrt(phi^2 + sigma_prime^2)
    double phi_star = std::sqrt(phi * phi + sigma_prime * sigma_prime);

    // 7) phi_new = 1 / sqrt(1/phi_star^2 + 1/v)
    double phi_new = 1.0 / std::sqrt(1.0 / (phi_star * phi_star) + 1.0 / v);

    // 8) mu_new = mu + phi_new^2 * sum_g_s_minus_E
    double mu_new = mu + (phi_new * phi_new) * sum_g_s_minus_E;

    // 9) Chuy?n ng??c v? MMR và RD
    player.mmr = mu_new * SCALE + 1500.0;
    player.mmr_deviation = phi_new * SCALE;
    player.volatility = sigma_prime;
}

// C?p nh?t c? tr?n: m?i ng??i dùng 5 ??i th? trong ??i ??i ph??ng
void update_match(std::vector<Player>& team_a, std::vector<Player>& team_b, bool a_wins) {
    // Chu?n hoá score cho t?ng c?p: n?u a_wins thì team A có score 1.0, team B 0.0
    double score_a = a_wins ? 1.0 : 0.0;
    double score_b = a_wins ? 0.0 : 1.0;

    // T?o records cho ??i B (???c dùng cho m?i player trong A)
    for (auto& p : team_a) {
        std::vector<OpponentRecord> opponents;
        opponents.reserve(team_b.size());
        for (const auto& opp : team_b) {
            opponents.push_back(OpponentRecord{opp.mmr, opp.mmr_deviation, score_a});
        }
        update_single(p, opponents);
    }

    // T??ng t? cho ??i B
    for (auto& p : team_b) {
        std::vector<OpponentRecord> opponents;
        opponents.reserve(team_a.size());
        for (const auto& opp : team_a) {
            opponents.push_back(OpponentRecord{opp.mmr, opp.mmr_deviation, score_b});
        }
        update_single(p, opponents);
    }
}

} // namespace Glicko2