#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <random>

using namespace std;

const int N = 6;

enum Difficulty {
    EASY,
    MEDIUM
};

const int FULL_MASK = (1 << N) - 1;

class Sudoku6{
public:
    int grid[N][N];
    int rowMask[N], colMask[N], boxMask[N];
    int boxH, boxW;
    mt19937 rng;

    Sudoku6(int h, int w) : boxH(h), boxW(w) {
        memset(grid, 0, sizeof(grid));
        memset(rowMask, 0, sizeof(rowMask));
        memset(colMask, 0, sizeof(colMask));
        memset(boxMask, 0, sizeof(boxMask));
        rng.seed(random_device{}());
    };

    inline int boxIndex(int r, int c) {
        return (r / boxH) * (N / boxW) + (c / boxW);
    }

    inline int candidatesMask(int r, int c) {
        int b = boxIndex(r, c);
        return FULL_MASK & ~(rowMask[r] | colMask[c] | boxMask[b]);
    }

    inline int popcount(int x) {
        return __builtin_popcount(x);
    }

    // === Validity Checker === //
    inline bool isValid(int r, int c, int num){
        int bit = 1 << (num - 1);
        int b = boxIndex(r, c);

        return !(rowMask[r] & bit) &&
               !(colMask[c] & bit) &&
               !(boxMask[b] & bit);
    }

    inline void place(int r, int c, int num) {
        int bit = 1 << (num - 1);
        int b = boxIndex(r, c);

        grid[r][c] = num;
        rowMask[r] |= bit;
        colMask[c] |= bit;
        boxMask[b] |= bit;
    }

    inline void remove(int r, int c, int num) {
        int bit = 1 << (num - 1);
        int b = boxIndex(r, c);

        grid[r][c] = 0;
        rowMask[r] ^= bit;
        colMask[c] ^= bit;
        boxMask[b] ^= bit;
    }

    // === Solved Grid Generation === //
    bool fillGrid(int r = 0, int c = 0) {
        if (r == N) return true;
        if (c == N) return fillGrid(r + 1, 0);
        if (grid[r][c] != 0) return fillGrid(r, c + 1);

        int mask = candidatesMask(r, c);
        vector<int> nums;

        while(mask) {
            int bit = mask & -mask;
            nums.push_back(__builtin_ctz(bit) + 1);
            mask ^= bit;
        }
        shuffle(nums.begin(), nums.end(), rng);
        
        for (int num : nums) {
            place(r, c, num);
            if (fillGrid(r, c + 1)) return true;
            remove(r, c, num);
        }
        return false;
    }

    // === MRV Cell Selection === //
    bool findMRVCell(int &r, int &c) {
        int best = N + 1;
        bool found = false;

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                if (grid[i][j] == 0) {
                    int cnt = popcount(candidatesMask(i, j));
                    if (cnt == 0) return false;

                    if (cnt < best) {
                        best = cnt;
                        r = i;
                        c = j;
                        found = true;
                        if (cnt == 1) return true;
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

        int mask = candidatesMask(r, c);
        while (mask){
            int bit = mask & -mask;
            int num = __builtin_ctz(bit) + 1;
            mask ^= bit;

            place(r, c, num);
            countSolutions(count, limit);
            remove(r, c, num);

            if (count >= limit) return;
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

        for (auto [r, c] : cells) {
            if (clues <= targetClues) break;

            int b = boxIndex(r, c);
            if (boxClues[b] <= ((diff == EASY) ? 3 : 2))
                continue;
        
            int backup = grid[r][c];
            remove(r, c, backup);

            if (hasUniqueSolution()) {
                boxClues[b]--;
                clues--;
            } else {
                place(r, c, backup);
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
