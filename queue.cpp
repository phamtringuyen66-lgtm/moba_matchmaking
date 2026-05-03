#include "queue.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>

void MatchQueue::addPlayer(const Player& p) {
    waitingList.push(p);
}

Player MatchQueue::popFront() {
    if (waitingList.empty()) {
        throw std::runtime_error("Hang doi trong!");
    }
    Player front = waitingList.top();
    waitingList.pop();
    return front;
}

int MatchQueue::getTopPlayers(Player* result, int n) const {
    int count = 0;
    std::priority_queue<Player, std::vector<Player>, CompareWait> temp = waitingList;

    while (!temp.empty() && count < n) {
        result[count] = temp.top();
        temp.pop();
        count++;
    }

    return count;
}

bool MatchQueue::isEmpty() const {
    return waitingList.empty();
}
int MatchQueue::size() const {
    return (int)waitingList.size();
}

void MatchQueue::printQueue() const {
    std::cout << "=== HANG DOI HIEN TAI (" << waitingList.size() << " nguoi) ===\n";
    std::priority_queue<Player, std::vector<Player>, CompareWait> temp = waitingList;

    int stt = 1;
    while (!temp.empty()) {
        std::cout << stt << ". ";
        printPlayer(temp.top());
        temp.pop();
        stt++;
    }

    std::cout << "=======================================\n";
}

double MatchQueue::getMmrRange(int waitTimeSeconds) const {
    // Khai báo h?ng s? theo yêu c?u
    constexpr double MMR_INITIAL_RANGE = 150.0;
    constexpr double MMR_EXPAND_PER_SEC = 1.67;
    constexpr double MMR_MAX_RANGE = 600.0;

    // B?o ??m th?i gian ch? không âm
    int t = std::max(0, waitTimeSeconds);

    // range(t) = min(MMR_INITIAL_RANGE + t * MMR_EXPAND_PER_SEC, MMR_MAX_RANGE)
    double range = MMR_INITIAL_RANGE + static_cast<double>(t) * MMR_EXPAND_PER_SEC;
    return std::min(range, MMR_MAX_RANGE);
}