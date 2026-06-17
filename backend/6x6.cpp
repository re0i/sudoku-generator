#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

using namespace std;

const int MAX_N = 12;
enum Difficulty {EASY, MEDIUM, HARD};

struct CandChange {
    int r, c;
    uint32_t oldMask;
};

struct SolveStats {
    int recursiveCalls  = 0;
    int guesses = 0;
    int backtracks = 0;
    int maxGuessDepth = 0;
    int maxBranching = 0;
    int nakedSingles = 0;
    int hiddenSingles = 0;
    int nakedPairs = 0;
};

class Sudoku {
public:
    int n;
    uint32_t fullMask;
    int grid[MAX_N][MAX_N];
    int solution[MAX_N][MAX_N];
    uint32_t rowMask[MAX_N], colMask[MAX_N], boxMask[MAX_N];
    uint32_t cand[MAX_N][MAX_N];
    int emptyCells;
    int boxH, boxW;
    mt19937 rng;
    vector<CandChange> candLog;

    Sudoku(int size, int h, int w) : n(size), boxH(h), boxW(w) {
        if (n < 1 || n > MAX_N) {
            throw invalid_argument("Unsupported grid size.");
        }
        if (boxH <= 0 || boxW <= 0 || boxH * boxW != n || n % boxH != 0 || n % boxW != 0) {
            throw invalid_argument("Unsupported box dimensions for grid size.");
        }

        fullMask = (1u << n) - 1u;
        clear();
        rng.seed(chrono::steady_clock::now().time_since_epoch().count());
    };

    void clear() {
        memset(grid, 0, sizeof(grid));
        memset(rowMask, 0, sizeof(rowMask));
        memset(colMask, 0, sizeof(colMask));
        memset(boxMask, 0, sizeof(boxMask));
        emptyCells = n * n;
        candLog.clear();
        initCandidates();
    }

    inline int boxIndex(int r, int c) {
        return (r / boxH) * (n / boxW) + (c / boxW);
    }

    inline void boxOrigin(int b, int &br, int &bc) const {
        int numBoxCols = n / boxW;
        br = (b / numBoxCols) * boxH;
        bc = (b % numBoxCols) * boxW;
    }

    void updateCandidatesInc(int r, int c, int num) {
        int bit = 1 << (num - 1);
        int br, bc;
        boxOrigin(boxIndex(r, c), br, bc);

        auto removeBit = [&](int rr, int cc) {
            if (cand[rr][cc] & bit) {
                candLog.push_back({rr, cc, cand[rr][cc]});
                cand[rr][cc] &= ~bit;
            }
        };

        for (int i = 0; i < n; i++) {
            if (grid[r][i] == 0) removeBit(r, i);
            if (grid[i][c] == 0) removeBit(i, c);
        }

        for (int i = 0; i < boxH; i++) 
            for (int j = 0; j < boxW; j++)
                removeBit(br + i, bc + j);

        candLog.push_back({r, c, cand[r][c]});
        cand[r][c] = 0;
    }

    // Counts how many bits are set to 1 in an integer (number of candidates)
    inline int popcount(int x) {
        return __builtin_popcount(x);
    }

    inline void place(int r, int c, int num) {
        if (num < 1 || num > n || grid[r][c] != 0) return;
        int bit = 1 << (num - 1);
        int b = boxIndex(r, c);

        grid[r][c] = num;
        emptyCells--;
        rowMask[r] |= bit;
        colMask[c] |= bit;
        boxMask[b] |= bit;
        updateCandidatesInc(r, c, num);
    }

    inline void undo(int checkpoint) {
        while ((int)candLog.size() > checkpoint) {
            auto ch = candLog.back();
            candLog.pop_back();
            cand[ch.r][ch.c] = ch.oldMask;
        }
    }

    inline void clearAll(int r, int c, int num) {
        if (num < 1 || num > n || grid[r][c] == 0) return;
        int bit = 1 << (num - 1);
        int b = boxIndex(r, c);

        grid[r][c] = 0;
        emptyCells++;
        rowMask[r] &= ~bit;
        colMask[c] &= ~bit;
        boxMask[b] &= ~bit;
    }

    void initCandidates() {
        for (int r = 0; r < n; r++) {
            for (int c = 0; c < n; c++) {
                if (grid[r][c] == 0) {
                    int b = boxIndex(r, c);
                    cand[r][c] = fullMask & ~rowMask[r] & ~colMask[c] & ~boxMask[b];
                } else {
                    cand[r][c] = 0;
                }
            }
        }
    }

    bool rebuildStateFromGrid() {
        memset(rowMask, 0, sizeof(rowMask));
        memset(colMask, 0, sizeof(colMask));
        memset(boxMask, 0, sizeof(boxMask));
        emptyCells = 0;
        candLog.clear();

        for (int r = 0; r < n; r++) {
            for (int c = 0; c < n; c++) {
                int num = grid[r][c];
                if (num == 0) {
                    emptyCells++;
                    continue;
                }
                if (num < 1 || num > n) return false;
                
                int bit = 1 << (num - 1);
                int b = boxIndex(r, c);
                if ((rowMask[r] & bit) || (colMask[c] & bit) || (boxMask[b] & bit)) {
                    return false;
                }
                rowMask[r] |= bit;
                colMask[c] |= bit;
                boxMask[b] |= bit;
            }
        }

        initCandidates();
        for (int r = 0; r < n; r++) {
            for (int c = 0; c < n; c++){
                if (grid[r][c] == 0 && cand[r][c] == 0) return false;
            }
        }
        return true;
    }

    bool fillGrid() {
        if (emptyCells == 0) return true;

        int r = -1, c = -1;
        if (!findMRVCell(r, c)) return false;

        int mask = cand[r][c];
        int nums[MAX_N], cnt = 0;

        while (mask) {
            int bit = mask & -mask;
            nums[cnt++] = __builtin_ctz(bit) + 1;
            mask ^= bit;
        }

        for (int i = cnt -1; i > 0; i--) {
            int j = rng() % (i + 1);
            swap(nums[i], nums[j]);
        }
        
        for (int i = 0; i < cnt; i++) {
            int num = nums[i];
            int cp = candLog.size();
            place(r, c, num);
            if (fillGrid()) return true;
            undo(cp);
            clearAll(r, c, num);
        }
        return false;
    }

    bool applyNakedPairs(SolveStats &s) {
        bool progress = false;

        // Naked pairs in rows
        for (int r = 0; r < n; r++) {
            for (int c1 = 0; c1 < n; c1++) {
                if (grid[r][c1] == 0 && popcount(cand[r][c1]) == 2) {
                    for (int c2 = c1 + 1; c2 < n; c2++) {
                        if (grid[r][c2] == 0 && cand[r][c1] == cand[r][c2]) {
                            int pairMask = cand[r][c1];
                            bool eliminated = false;
                            for (int c = 0; c < n; c++) {
                                if (c != c1 && c != c2 && grid[r][c] == 0 && (cand[r][c] & pairMask)) {
                                    candLog.push_back({r, c, cand[r][c]});
                                    cand[r][c] &= ~pairMask;
                                    eliminated = true;
                                }                              
                            }
                            if (eliminated) {
                                s.nakedPairs++;
                                progress = true;
                            }
                        }
                    }
                }
            }
        }

        //Naked Pairs in columns
        for (int c = 0; c < n; c++) {
            for (int r1 = 0; r1 < n; r1++) {
                if (grid[r1][c] == 0 && popcount(cand[r1][c]) == 2) {
                    for (int r2 = r1 + 1; r2 < n; r2++) {
                        if (grid[r2][c] == 0 && cand[r1][c] == cand[r2][c]) {
                            int pairMask = cand[r1][c];
                            bool eliminated = false;
                            for (int r = 0; r < n; r++) {
                                if (r != r1 && r != r2 && grid[r][c] == 0 && (cand[r][c] & pairMask)) {
                                    candLog.push_back({r, c, cand[r][c]});
                                    cand[r][c] &= ~pairMask;
                                    eliminated = true;
                                }
                            }
                            if (eliminated) {
                                s.nakedPairs++;
                                progress = true;
                            }
                        }
                    }
                }
            }
        }

        // Naked pairs in boxes
        for (int b = 0; b < n; b++) {
            int br, bc;
            boxOrigin(b, br, bc);
            vector<pair<int, int>> cells;
            for (int i = 0; i < boxH; i++) {
                for (int j = 0; j < boxW; j++) {
                    int r = br + i, c = bc + j;
                    if (grid[r][c] == 0) {
                        cells.emplace_back(r, c);
                    }
                }
            }

            for (size_t i = 0; i < cells.size(); i++) {
                auto [r1, c1] = cells[i];
                if (popcount(cand[r1][c1]) == 2) {
                    for (size_t j = i + 1; j < cells.size(); j++) {
                        auto [r2, c2] = cells[j];
                        if (cand[r1][c1] == cand[r2][c2]) {
                            int pairMask = cand[r1][c1];
                            bool eliminated = false;
                            for (size_t k = 0; k < cells.size(); k++) {
                                if (k != i && k != j) {
                                    auto [r3, c3] = cells[k];
                                    if (grid[r3][c3] == 0 && cand[r3][c3] & pairMask) {
                                        candLog.push_back({r3, c3, cand[r3][c3]});
                                        cand[r3][c3] &= ~pairMask;
                                        eliminated = true;
                                    }
                                }
                            }
                            if (eliminated) {
                                s.nakedPairs++;
                                progress = true;
                            }
                        }
                    }
                }
            }
        }
        return progress;
    }

    // MRV Cell Selection
    bool findMRVCell(int &r, int &c) {
        int best = n + 1;
        bool found = false;

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (grid[i][j] == 0) {
                    int cnt = popcount(cand[i][j]);
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
    
    bool countSolutions(int &count, int limit) {
        if (count >= limit) return true;

        int r = -1, c = -1;
        int best = n + 1;

        // Proper MRV selection
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (grid[i][j] == 0) {
                    int pc = popcount(cand[i][j]);
                    if (pc == 0) return false;   // dead branch

                    if (pc < best) {
                        best = pc;
                        r = i;
                        c = j;
                        if (pc == 1) break;
                    }
                }
            }
        }

        if (r == -1) {      // solved
            count++;
            return count >= limit;
        }

        int mask = cand[r][c];

        while (mask) {
            int bit = mask & -mask;
            mask ^= bit;
            int num = __builtin_ctz(bit) + 1;

            int cp = candLog.size();
            place(r, c, num);

            if (countSolutions(count, limit)) {
                undo(cp);
                clearAll(r, c, num);
                return true;
            }

            undo(cp);
            clearAll(r, c, num);
        }
        return false;
    }

    bool hasUniqueSolution() {
        Sudoku copy(n, boxH, boxW);
        memcpy(copy.grid, grid, sizeof(grid));
        if (!copy.rebuildStateFromGrid()) return false;
        int count = 0;
        copy.countSolutions(count, 2);
        return count == 1;
    }

    bool applyNakedSingles(SolveStats &stats) {
        bool progress = false;
        for (int r = 0; r < n; r++) {
            for (int c = 0; c < n; c++) {
                if (grid[r][c] == 0) {
                    int mask = cand[r][c];
                    if (popcount(mask) == 1) {
                        int num = __builtin_ctz(mask) + 1;
                        place(r, c, num);
                        stats.nakedSingles++;
                        progress = true;
                    }
                }
            }
        }
        return progress;
    }

    bool applyHiddenSingles(SolveStats &s) {
        bool progress = false;
        // Check rows
        for (int r = 0; r < n; r++) {
            for (int num = 1; num <= n; num++) {
                int bit = 1 << (num - 1);
                int count = 0, pos = -1;
                for (int c = 0; c < n; c++) {
                    if (grid[r][c] == 0 && (cand[r][c] & bit)) {
                        count++, pos = c;
                    }
                }
                if (count == 1) {
                    place(r, pos, num);
                    s.hiddenSingles++;
                    progress = true;
                }
            }
        }
        // Check columns
        for (int c = 0; c < n; c++) {
            for (int num = 1; num <= n; num++) {
                int bit = 1 << (num - 1);
                int count = 0, pos = -1;
                for (int r = 0; r < n; r++) {
                    if (grid[r][c] == 0 && (cand[r][c] & bit)) {
                        count++;
                        pos = r;
                    }
                }
                if (count == 1) {
                    place(pos, c, num);
                    s.hiddenSingles++;
                    progress = true;
                }
            }
        }
        // Check boxes
        for (int b = 0; b < n; b++) {
            int br, bc;
            boxOrigin(b, br, bc);
            for (int num = 1; num <= n; num++) {
                int bit = 1 << (num - 1);
                int count = 0, pr = -1, pc = -1;
                for (int i = 0; i < boxH; i++) {
                    for (int j = 0; j < boxW; j++) {
                        int r = br + i, c = bc + j;
                        if (grid[r][c] == 0 && (cand[r][c] & bit)) {
                            count++;
                            pr = r;
                            pc = c;
                        }
                    }
                }
                if (count == 1) {
                    place(pr, pc, num);
                    s.hiddenSingles++;
                    progress = true;
                }
            }
        }
        return progress;
    }

    bool solveStats(SolveStats &s, int depth = 0) {
        s.recursiveCalls++;
        s.maxGuessDepth = max(s.maxGuessDepth, depth);

        // Apply human techniques
        bool progress = true;
        while (progress) {
            progress = false;
            progress |= applyNakedSingles(s);
            progress |= applyHiddenSingles(s);
            progress |= applyNakedPairs(s);
        }

        if (emptyCells == 0) return true;

        // If not solved, guess
        s.guesses++;

        int r, c;
        if (!findMRVCell(r, c)) return false;

        int mask = cand[r][c];
        int choices = popcount(mask);
        s.maxBranching = max(s.maxBranching, choices);

        while (mask) {
            int bit = mask & -mask;
            int num = __builtin_ctz(bit) + 1;
            mask ^= bit;

            int cp = candLog.size();
            place(r, c, num);

            if (solveStats(s, depth + 1)) 
                return true;

            undo(cp);
            clearAll(r, c, num);

            s.backtracks++;
        }
        return false;
    }

    bool matchesDifficulty(Difficulty diff) {
        Sudoku copy(n, boxH, boxW);
        memcpy(copy.grid, grid, sizeof(grid));
        if (!copy.rebuildStateFromGrid()) return false;
        SolveStats s;
        copy.solveStats(s);

        if (n == 4) {
            if (diff == EASY) return s.guesses == 0;
            if (diff == MEDIUM) return s.guesses <= 2;
            if (diff == HARD) return s.guesses >= 1;
        } else if (n == 6) {
            if (diff == EASY) {
            return s.guesses <= 1 &&
                   s.backtracks <= 2 &&
                   s.nakedSingles + s.hiddenSingles >= 10;
            }

            if (diff == MEDIUM) {
                return s.guesses <= 5 &&
                       s.backtracks <= 10 &&
                       s.nakedSingles + s.hiddenSingles >= 5 &&
                       s.maxGuessDepth <= 2;
            }

            if (diff == HARD) {
                int clues = n * n - emptyCells;
                return clues <= 10 &&
                       s.maxGuessDepth >= 1 &&
                       (s.guesses >= 1 || s.backtracks >= 2);
            }
        } else if (n == 9) {
            if (diff == EASY)
                return s.guesses <= 1 && s.backtracks <= 2 &&
                    s.nakedSingles + s.hiddenSingles >= 20;
            if (diff == MEDIUM)
                return s.guesses <= 8 && s.backtracks <= 15 &&
                    s.nakedSingles + s.hiddenSingles >= 10 &&
                    s.maxGuessDepth <= 3;
            if (diff == HARD) {
                int clues = n * n - emptyCells;
                return clues <= 25 && s.maxGuessDepth >= 2 &&
                    (s.guesses >= 3 || s.backtracks >= 5);
            }
        } else if (n == 12) {
            if (diff == EASY)
                return s.guesses <= 2 && s.backtracks <= 4 &&
                    s.nakedSingles + s.hiddenSingles >= 40;
            if (diff == MEDIUM)
                return s.guesses <= 12 && s.backtracks <= 20 &&
                    s.nakedSingles + s.hiddenSingles >= 20 &&
                    s.maxGuessDepth <= 4;
            if (diff == HARD)
                return (n * n - emptyCells) <= 40 &&
                    s.maxGuessDepth >= 3 &&
                    (s.guesses >= 5 || s.backtracks >= 8);
        }
        return false;
    }

    bool generatePuzzle(Difficulty diff, int maxAttempts = 2000) {
        for (int attempts = 0; attempts < maxAttempts; attempts++) {
            clear();
            fillGrid();
            memcpy(solution, grid, sizeof(grid));
            initCandidates();
            candLog.clear();

            int targetClues;

            if (n == 4) targetClues = (diff == EASY ? 10 : (diff == MEDIUM ? 9 : 8));
            else if (n == 6) targetClues = (diff == EASY ? 24 : (diff == MEDIUM ? 18 : 10));
            else if (n == 9) targetClues = (diff == EASY ? 36 : (diff == MEDIUM ? 27 : 22));
            else if (n == 12) targetClues = (diff == EASY ? 55 : (diff == MEDIUM ? 44 : 33));
            else targetClues = n * n / 2;

            int clues = n * n;
            vector<pair<int, int>> cells;
            for (int r = 0; r < n; r++)
                for (int c = 0; c < n; c++)
                    cells.emplace_back(r, c);

            shuffle(cells.begin(), cells.end(), rng);

            for (auto [r, c] : cells) {
                if (clues <= targetClues) break;

                int backup = grid[r][c];
                if (backup == 0) continue;
                clearAll(r, c, backup);
                initCandidates();
                candLog.clear();

                if (!hasUniqueSolution()) {
                    place(r, c, backup);
                    initCandidates();
                    candLog.clear();
                } else {
                    clues--;
                }
                candLog.clear();
            }

            if (hasUniqueSolution() && matchesDifficulty(diff)) {
                candLog.clear();
                return true;
            }
        }
        return false;
    }

    string difficultyToString(Difficulty diff) {
        switch (diff) {
            case EASY: return "an easy";
            case MEDIUM: return "a medium";
            case HARD: return "a hard";
            default: return "an unknown";
        }
    }

    static char cellChar(int v) {
        if (v == 0)  return '.';
        if (v <= 9)  return '0' + v;
        return 'A' + (v - 10);
    }

    void printPuzzle() {
        bool wide = (n > 9);
        for (int r = 0; r < n; r++) {
            for (int c = 0; c < n; c++) {
                if (wide) cout << ' ';
                cout << cellChar(grid[r][c]) << ' ';
                if ((c + 1) % boxW == 0 && c != n - 1) cout << "| ";
            }
            cout << "\n";
            if ((r + 1) % boxH == 0 && r != n - 1) {
                int dashes = wide ? (3 * n + 3 * (n / boxW - 1))
                                : (2 * n + 3 * (n / boxW - 1));
                for (int k = 0; k < dashes; k++) cout << '-';
                cout << "\n";
            }
        }
    }

    void printSol() {
        cout << "\n";
        bool wide = (n > 9);
        for (int r = 0; r < n; r++) {
            for (int c = 0; c < n; c++) {
                if (wide) cout << ' ';
                cout << cellChar(solution[r][c]) << ' ';
                if ((c + 1) % boxW == 0 && c != n - 1) cout << "| ";
            }
            cout << "\n";
            if ((r + 1) % boxH == 0 && r != n - 1) {
                int dashes = wide ? (3 * n + 3 * (n / boxW - 1))
                                : (2 * n + 3 * (n / boxW - 1));
                for (int k = 0; k < dashes; k++) cout << '-';
                cout << "\n";
            }
        }
    }
};

int main() {
    int size = 12;
    int boxH = 4, boxW = 3;
    Difficulty diff = EASY;

    Sudoku sudoku(size, boxH, boxW);
    if (!sudoku.generatePuzzle(diff)) {
        cerr << "Failed to generate " << sudoku.difficultyToString(diff) << " " << size << "x" << size <<" puzzle after max attempts.\n";
        return 1;
    }

    sudoku.printPuzzle();
    sudoku.printSol();
    return 0;
}