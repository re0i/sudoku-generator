#include <iostream>
#include <algorithm>
#include <random>
#include <vector>
#include <cmath>

using namespace std;

const int N = 4;

int boxSize(int N){
    return static_cast<int>(sqrt(N));
}

bool isValid(const vector<vector<int>>& grid, int row, int col, int num){
    for (int x = 0; x < N; x++){
        if (grid[row][x] == num) return false;
        if (grid[x][col] == num) return false;
    }

    int startRow = row - row % boxSize(N);
    int startCol = col - col % boxSize(N);

    for (int r = 0; r < boxSize(N); r++)
        for (int c = 0; c < boxSize(N); c++)
            if (grid[startRow + r][startCol + c] == num) return false;

    return true;
}

bool fillGrid(vector<vector<int>>& grid){
    for (int r = 0; r < N; r++){
        for (int c = 0; c < N; c++){
            if (grid[r][c] == 0){

                vector<int> nums = {1, 2, 3, 4};
                shuffle(nums.begin(), nums.end(), default_random_engine(random_device{}()));

                for (int num : nums){
                    if (isValid(grid, r, c, num)){
                        grid[r][c] = num;
                        
                        if (fillGrid(grid)) return true;

                        grid[r][c] = 0;
                    }
                }
                return false;
            }
        }
    }
    return true;
}

int countSol(vector<vector<int>> grid){
    for (int r = 0; r < N; r++){
        for (int c = 0; c < N; c++){
            if (grid[r][c] == 0){
                int solutions = 0;
                for (int num = 1; num <= N; num++){
                    if (isValid(grid, r, c, num)){
                        grid[r][c] = num;

                        int count = countSol(grid);

                        solutions += count;
                        if (solutions > 1) return 2;
                        grid[r][c] = 0;
                    }
                }
                return solutions;
            }
        }
    }
    return 1;
}

void makePuzzle(vector<vector<int>>& grid){
    vector<pair<int,int>> cells;
    for (int r = 0; r < N; r++)
        for (int c = 0; c < N; c++)
            cells.push_back({r, c});
    
    shuffle(cells.begin(), cells.end(), default_random_engine(random_device{}()));

    for (auto& cell: cells){
        int r = cell.first;
        int c = cell.second;

        int backup = grid[r][c];
        grid[r][c] = 0;

        if (countSol(grid) != 1) grid[r][c] = backup;
    }
}

void printGrid(const vector<vector<int>>& grid){
    for (int r = 0; r < N; r++){
        for (int c = 0; c < N; c++)
            cout << (grid[r][c] ? char('0' + grid[r][c]) : '.') << ' ';
        cout << '\n';
    }
}

int sudoku4(){
    vector<vector<int>> grid(N, vector<int>(N, 0));

    fillGrid(grid);
    makePuzzle(grid);
    cout << "\nPuzzle:\n";
    printGrid(grid);

    fillGrid(grid);
    cout << "Solution:\n";
    printGrid(grid);

    return 0;
}

int main(){
    vector<vector<int>> grid(N, vector<int>(N, 0));

    fillGrid(grid);
    makePuzzle(grid);
    cout << "\nPuzzle:\n";
    printGrid(grid);

    fillGrid(grid);
    cout << "Solution:\n";
    printGrid(grid);

    return 0;
}