#include "hungarian.h"
#include <vector>
#include <limits>
#include <cmath>
#include <algorithm>

namespace hungarian {

    std::optional<Assignment> HungarianSolver::solve(const CostMatrix& cost_matrix) {
        // Kiểm tra hợp lệ cơ bản
        if (!validate(cost_matrix)) return std::nullopt;

        Assignment result;
        result.total_cost = 0.0;
        for (std::size_t i = 0; i < kN; ++i) result.roles[i] = -1;

        const int n = static_cast<int>(kN);
        const double INF = 1e18;

        // Ma trận 1-based để thuận tiện cho thuật toán
        std::vector<std::vector<double>> a(n + 1, std::vector<double>(n + 1, 0.0));
        for (int i = 1; i <= n; ++i) {
            for (int j = 1; j <= n; ++j) {
                a[i][j] = cost_matrix[i - 1][j - 1];
            }
        }

        // Potentials and matching arrays
        std::vector<double> u(n + 1, 0.0), v(n + 1, 0.0);
        std::vector<int> p(n + 1, 0), way(n + 1, 0);

        for (int i = 1; i <= n; ++i) {
            p[0] = i;
            int j0 = 0;
            std::vector<double> minv(n + 1, INF);
            std::vector<char> used(n + 1, false);
            std::fill(way.begin(), way.end(), 0);

            while (true) {
                used[j0] = true;
                int i0 = p[j0];
                double delta = INF;
                int j1 = 0;

                for (int j = 1; j <= n; ++j) {
                    if (used[j]) continue;
                    double cur = a[i0][j] - u[i0] - v[j];
                    if (cur < minv[j]) {
                        minv[j] = cur;
                        way[j] = j0;
                    }
                    if (minv[j] < delta) {
                        delta = minv[j];
                        j1 = j;
                    }
                }

                for (int j = 0; j <= n; ++j) {
                    if (used[j]) {
                        u[p[j]] += delta;
                        v[j] -= delta;
                    } else {
                        minv[j] -= delta;
                    }
                }

                j0 = j1;
                if (p[j0] == 0) break;
            }

            // Augmenting path
            do {
                int j1 = way[j0];
                p[j0] = p[j1];
                j0 = j1;
            } while (j0 != 0);
        }

        // p[j] = i matched to column j
        std::vector<int> row_to_col(n + 1, 0);
        for (int j = 1; j <= n; ++j) {
            if (p[j] != 0) row_to_col[p[j]] = j;
        }

        // Chuyển kết quả về 0-based và tính tổng chi phí
        double total = 0.0;
        for (int i = 1; i <= n; ++i) {
            int col = row_to_col[i];
            if (col >= 1 && col <= n) {
                result.roles[i - 1] = col - 1;
                total += cost_matrix[i - 1][col - 1];
            } else {
                result.roles[i - 1] = -1;
            }
        }

        result.total_cost = total;
        return result;
    }

    bool HungarianSolver::validate(const CostMatrix& cost_matrix) {
        // Ma trận cố định kN x kN, chỉ kiểm tra tính hợp lệ số học
        if (cost_matrix.size() != kN) return false;
        for (std::size_t i = 0; i < kN; ++i) {
            for (std::size_t j = 0; j < kN; ++j) {
                double v = cost_matrix[i][j];
                if (!std::isfinite(v)) return false;
            }
        }
        return true;
    }

} // namespace hungarian