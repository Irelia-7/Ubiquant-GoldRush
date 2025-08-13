#include <random>
#include <utility> // for std::pair
#include <iostream>
#include <string>
#include <vector>
#include <thread>   // for std::this_thread::sleep_for
#include <chrono>   // for std::chrono::seconds

class Player
{
public:
    Player() {}

    int* MoveDecision(const std::vector<std::vector<int>> &grid, int gold, int gold_2)
    {
        return this->data; // 返回默认的移动决策
    }

    static Player& getInstance()
    {
        static Player instance; // 确保只有一个实例
        return instance;
    }

private:
    std::pair<int, int> position;
    int gold;
    int action;
    int data[3] = {4, 4, 4}; // 默认的移动决策
};

extern "C" {
    int* moveDecision(const std::vector<std::vector<int>> &grid, int gold, int gold_2) {
        return Player::getInstance().MoveDecision(grid, gold, gold_2);
    }
}