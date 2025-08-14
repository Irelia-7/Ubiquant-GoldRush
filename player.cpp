#include <random>
#include <utility> // for std::pair
#include <iostream>
#include <string>
#include <vector>
#include <thread>   // for std::this_thread::sleep_for
#include <chrono>   // for std::chrono::seconds
#include <cmath>
#include <algorithm>

int MAX_DEPTH = 3;
int TIME_LIMIT_MS = 1000;

class Player
{
public:
    Player(): rng(std::random_device{}()), dist(0, 4){} // Initialize random number generator and distribution

    // left, right, up, down, keep still
    const int dx[5] = {-1, 1, 0, 0, 0};
    const int dy[5] = {0, 0, 1, -1, 0};
    int round = 0;

    struct Node {
        int visits = 0;
        int total_reward = 0;
        std::vector<int> path;
        std::vector<Node*> children;
        Node* parent = nullptr;
    };

    static Player& getInstance()
    {
        static Player instance; // Ensure only one instance exists
        return instance;
    }

    int* MoveDecision(const std::vector<std::vector<int>> &grid, int gold, int gold_2)
    {
        round++;
        // 0. Find my position
        if (round == 1) {
            if (grid[0][0] == -9) {
                this->position = {0, 0};
            }
            else if (grid[0][16] == -9) {
                this->position = {0, 16};
            }
            else if (grid[16][0] == -9) {
                this->position = {16, 0};
            }
            else if (grid[16][16] == -9) {
                this->position = {16, 16};
            }
        }
        else {
            // Iterate over all 8 combinations of executing or not executing the 3 actions
            for (int mask = 0; mask < 8; ++mask) {
                int x = this->position.first;
                int y = this->position.second;
                for (int i = 0; i < 3; ++i) {
                    if (mask & (1 << i)) {
                        x += this->dx[this->data[i]];
                        y += this->dy[this->data[i]];
                    }
                }
                if (grid[x][y] == -9) {
                    this->position = {x, y};
                    break;
                }
            }
        }

        // // 1. Create the root node
        // Node root;
        // root.path = {};

        // // 2. expand the MCTS
        // expand(root);

        // // 3. Selection

        // // 4. Rollout

        // // 5. Backpropagation
        Node root;
        root.path = {};

        auto start = std::chrono::high_resolution_clock::now();

        while (true) {
            auto now = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration<double, std::milli>(now - start).count() > TIME_LIMIT_MS)
                break;

            Node* node = select(&root);
            if (node->path.size() < MAX_DEPTH)
                expand(node);

            Node* leaf = node->children.empty() ? node : node->children[rng() % node->children.size()];
            if (leaf->path.size() == MAX_DEPTH) {
                double reward = simulate(grid, this->position.first, this->position.second, gold, leaf->path);
                backpropagate(leaf, reward);
            }
        }

        Node* best = nullptr;
        double best_score = -1;
        for (auto* child : root.children) {
            if (child->visits == 0) continue;
            double avg = child->total_reward / child->visits;
            if (avg > best_score) {
                best_score = avg;
                best = child;
            }
        }

        // 输出最优路径前 MAX_DEPTH 步
        for (int i = 0; i < MAX_DEPTH; ++i)
            this->data[i] = (best && i < best->path.size()) ? best->path[i] : -1;

        return this->data;


        // return this->data; // 返回默认的移动决策
    }


private:
    // Tools
    std::mt19937 rng;
    std::uniform_int_distribution<int> dist;

    // Attributes
    std::pair<int, int> position;
    int gold;
    int action;
    int data[3] = {4, 4, 4}; // Default move decisions

    // Functions 

    bool inBounds(int x, int y) {
        return x >= 0 && x < 17 && y >= 0 && y < 17;
    }

    bool inCentral(int x, int y) {
        return x >= 4 && x <= 12 && y >= 4 && y <= 12;
    }

    void expand(Node* node) {
        if (node->path.size() >= MAX_DEPTH) return;
        for (int d = 0; d < 4; ++d) {
            Node* child = new Node();
            child->path = node->path;
            child->path.push_back(d);
            child->parent = node;
            node->children.push_back(child);
        }
    }

    // To be completed
    Node* select(Node* node) {
        while (!node->children.empty()) {
            node = *std::max_element(node->children.begin(), node->children.end(), [&](Node* a, Node* b) {
                return ucb(a) < ucb(b);
            });
        }
        return node;
    }

    // To be completed
    double ucb(Node* node) {
        if (node->visits == 0) return 1e9;
        double avg = node->total_reward / node->visits;
        double log_N = std::log(node->parent->visits + 1);
        return avg + 1.4 * std::sqrt(log_N / node->visits);
    }

    // To be completed
    double simulate(const std::vector<std::vector<int>>& grid, int sx, int sy, int gold, const std::vector<int>& path) {
        int x = sx, y = sy;
        int score = 0, g = gold;

        for (int dir : path) {
            int nx = x + dx[dir], ny = y + dy[dir];
            if (!inBounds(nx, ny) || grid[nx][ny] == -1 || grid[nx][ny] == -2) continue;
            x = nx; y = ny;
            if (grid[x][y] >= 1)
                score += grid[x][y];
            else if (grid[x][y] == -3) {
                // int loss = (g * 30 + 99) / 100;
                int loss = static_cast<int>(std::ceil(g * 0.1));
                score -= loss;
                g = std::max(0, g - loss);
            } else if (grid[x][y] == 0 && x >= 4 && x <= 12 && y >= 4 && y <= 12) {
                score += 2;
            }
        }

        return static_cast<double>(score);
    }

    void backpropagate(Node* node, int reward) {
        while (node) {
            node->visits++;
            node->total_reward += reward;
            node = node->parent;
        }
    }

};

extern "C" {
    int* moveDecision(const std::vector<std::vector<int>> &grid, int gold, int gold_2) {
        return Player::getInstance().MoveDecision(grid, gold, gold_2);
    }
}