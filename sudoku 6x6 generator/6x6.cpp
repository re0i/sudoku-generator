#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <random>
#include <bits/stdc++.h>

using namespace std;

const int N = 6;

enum Difficulty {
    EASY,
    MEDIUM
};

class Sudoku6{
public:
    int grid[N][N];
    int boxH, boxW;
    mt19937 rng;

    Sudoku6(int h, int w) : boxH(h), boxW(w) {
        memset(grid, 0, sizeof(grid));
        rng.seed(random_device{}());
    };

    // === Validity Checker === //
    bool isValid(int r, int c, int num){
        // Row & Column
        for (int i = 0; i < N; i++) {
            if (grid[r][i] == num) return false;
            if (grid[i][c] == num) return false;
        }

        // Box
        int br = (r / boxH) * boxH;
        int bc = (c / boxW) * boxW;

        for (int i = 0; i < boxH; i++) {
            for (int j = 0; j < boxW; j++) {
                if (grid[br + i][bc + j] == num) return false;
            }
        }
        return true;
    }

    // === Solved Grid Generation === //
    bool fillGrid(int r = 0, int c = 0) {
        if (r == N) return true;
        if (c == N) return fillGrid(r + 1, 0);
        if (grid[r][c] != 0) return fillGrid(r, c + 1);

        vector<int> nums = {1, 2, 3, 4, 5, 6};
        shuffle(nums.begin(), nums.end(), rng);
        
        for (int num : nums) {
            if (isValid(r, c, num)){
                grid[r][c] = num;
                if (fillGrid(r, c + 1)) return true;
                grid[r][c] = 0;
            }
        }
        return false;
    }

    // === MRV Cell Selection === //
    bool findMRVCell(int &r, int &c) {
        int minCount = N + 1;
        bool found = false;

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (grid[i][j] == 0) {
                    int cnt = 0;
                    for (int num = 1; num <= N; num++)
                        if (isValid(i, j, num))
                            cnt++;

                    if (cnt < minCount) {
                        minCount = cnt;
                        r = i;
                        c = j;
                        found = true;
                    }
                }
            }
        }
        return found;
    }

    // === Solution Counter === //
    void countSolutions(int &count, int limit = 2) {
        if (count >= limit) return;

        int r, c;
        if (!findMRVCell(r, c)) {
            count++;
            return;
        }

        for (int num = 1; num <= N; num++) {
            if (isValid(r, c, num)){
                grid[r][c] = num;
                countSolutions(count, limit);
                grid[r][c] = 0;
            }
        }
    }

    bool hasUniqueSolution() {
        Sudoku6 copy = *this;
        int count = 0;
        copy.countSolutions(count, 2);
        return count == 1;
    }

    // === Puzzle Generation === //
    void generatePuzzle(Difficulty diff) {
        fillGrid();

        int targetClues = (diff == EASY) ? 24 : 18;
        int clues = N * N;

        vector<pair<int, int>> cells;
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                cells.emplace_back(i, j);

        shuffle(cells.begin(), cells.end(), rng);

        int boxCount = (N / boxH) * (N / boxW);
        vector<int> boxClues(boxCount, boxH * boxW);

        auto boxIndex = [&](int r, int c) {
            return (r / boxH) * (N / boxW) + (c / boxW);
        };

        for (auto [r, c] : cells) {
            if (clues <= targetClues) break;

            int b = boxIndex(r, c);
            if (boxClues[b] <= ((diff == EASY) ? 3 : 2))
                continue;
        
            int backup = grid[r][c];
            grid[r][c] = 0;

            if (hasUniqueSolution()) {
                boxClues[b]--;
                clues--;
            } else {
                grid[r][c] = backup;
            }
        }
    }

    // === Print === //
    void print() {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                cout << (grid[i][j] == 0 ? "." : to_string(grid[i][j])) << " ";
                if ((j + 1) % boxW == 0 && j != N - 1) cout << "| ";
            }
            cout << "\n";
            if ((i + 1) % boxH == 0 && i != N - 1) {
                for (int k = 0; k < N + 2; k++) cout << "--";
                cout << "\n";
            }
        }
    }
};

// === Main === //
int main() {
    Sudoku6 sudoku(2, 3);
    sudoku.generatePuzzle(EASY);

    sudoku.print();
    return 0;
}