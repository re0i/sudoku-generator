#include <iostream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <random>

using namespace std;

const int N = 6;

enum Difficulty {
    EASY,
    MEDIUM,
    HARD
};

const int FULL_MASK = (1 << N) - 1;

// === Solution Stats === //
struct SolveStats {
    int recursiveCalls  = 0;
    int forcedMoves = 0;
    int maxBranching = 0;
};

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

    // Computes the box index for a cell (r, c) based on box dimensions
    inline int boxIndex(int r, int c) {
        return (r / boxH) * (N / boxW) + (c / boxW);
    }

    // Returns a bitmask of all valid candidate numbers for cell (r, c)
    inline int candidatesMask(int r, int c) {
        int b = boxIndex(r, c);
        return FULL_MASK & ~(rowMask[r] | colMask[c] | boxMask[b]);
    }

    // Counts how many bits are set to 1 in an integer (number of candidates)
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

    // Places a number in the grid and updates row/col/box masks
    inline void place(int r, int c, int num) {
        int bit = 1 << (num - 1);
        int b = boxIndex(r, c);

        grid[r][c] = num;
        rowMask[r] |= bit;
        colMask[c] |= bit;
        boxMask[b] |= bit;
    }

    // Removes a number from the grid and restores row/col/box masks
    inline void remove(int r, int c, int num) {
        int bit = 1 << (num - 1);
        int b = boxIndex(r, c);

        grid[r][c] = 0;
        rowMask[r] &= ~bit;
        colMask[c] &= ~bit;
        boxMask[b] &= ~bit;
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

    // === Solve with statistics === //
    bool solveStats(SolveStats &stats) {
        stats.recursiveCalls++;
        
        int r, c;
        if (!findMRVCell(r, c)) return true;

        int mask = candidatesMask(r, c);
        int choices = popcount(mask);

        stats.maxBranching = max(stats.maxBranching, choices);
        if (choices == 1) stats.forcedMoves++;

        while (mask) {
            int bit = mask & -mask;
            int num = __builtin_ctz(bit) + 1;
            mask ^= bit;

            place(r, c, num);
            if (solveStats(stats)) return true;
            remove(r, c, num);
        }
        return false;
    }

    // Determines if the puzzle matches the requested difficulty level
    bool matchesDifficulty(Difficulty diff) {
        Sudoku6 copy = *this;
        SolveStats stats;
        copy.solveStats(stats);

        if (diff == EASY) {
            return stats.maxBranching <= 2 &&
                   stats.recursiveCalls < 300 &&
                   stats.forcedMoves > 10;
        }

        if (diff == MEDIUM) {
            return stats.maxBranching <= 3 &&
                   stats.recursiveCalls < 1500 &&
                   stats.forcedMoves >= 4;
        }

        if (diff == HARD) {
            return stats.maxBranching >= 3 &&
                   stats.recursiveCalls >= 300 &&
                   stats.forcedMoves <= 6;
        }

        return false;
    }
    // === Puzzle Generation === //
    void generatePuzzle(Difficulty diff) {
        fillGrid();

        int targetClues;
        if (diff == EASY) targetClues = 24;
        if (diff == MEDIUM) targetClues = 18;
        if (diff == HARD) targetClues = 14;

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
            int minBoxClues = (diff == EASY) ? 3 : (diff == MEDIUM) ? 2 : 1;
            if (boxClues[b] <= minBoxClues)
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
        int attempts = 0;
        const int maxAttempts = 10;
        while (!matchesDifficulty(diff) && attempts < maxAttempts) {
            attempts++;
            // Reset everything
            memset(grid, 0, sizeof(grid));
            memset(rowMask, 0, sizeof(rowMask));
            memset(colMask, 0, sizeof(colMask));
            memset(boxMask, 0, sizeof(boxMask));

            fillGrid();

            // Recreate the list of cells and remove clues again
            int clues = N * N;
            vector<pair<int, int>> cells;
            for (int i = 0; i < N; i++)
                for (int j = 0; j < N; j++)
                    cells.emplace_back(i, j);

            shuffle(cells.begin(), cells.end(), rng);

            int targetClues = (diff == EASY ? 24 : diff == MEDIUM ? 18 : 14);
            int boxCount = (N / boxH) * (N / boxW);
            vector<int> boxClues(boxCount, boxH * boxW);

            for (auto [r, c] : cells) {
                if (clues <= targetClues) break;

                int b = boxIndex(r, c);
                int minBoxClues = (diff == EASY) ? 3 : (diff == MEDIUM) ? 2 : 1;
                if (boxClues[b] <= minBoxClues)
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
    sudoku.generatePuzzle(HARD);

    sudoku.print();
    return 0;
};
