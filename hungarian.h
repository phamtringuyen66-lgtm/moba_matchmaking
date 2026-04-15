#pragma once

#include <array>
#include <optional>
#include <cstddef>

namespace hungarian {

constexpr std::size_t kN = 5;
using CostMatrix = std::array<std::array<double, kN>, kN>;
using AssignmentArray = std::array<int, kN>;

struct Assignment {
    AssignmentArray roles;
    double total_cost = 0.0;
};

class HungarianSolver {
public:
    static std::optional<Assignment> solve(const CostMatrix& cost_matrix);
    static bool validate(const CostMatrix& cost_matrix);

private:
    HungarianSolver() = delete;
};

}