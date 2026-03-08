#include "crow_all.h"
#include <asio.hpp>
#include "6x6.cpp"

int main() {
    crow::SimpleApp app;
    CROW_ROUTE(app, "/")
    ([](){
        return "Sudoku server is running!";
    });

    CROW_ROUTE(app, "/sudoku4")
    ([](){
        int size = 4;
        int boxH = 2, boxW = 2;
        Difficulty diff = MEDIUM;

        Sudoku6 sudoku(size, boxH, boxW);
        if (!sudoku.generatePuzzle(diff)) {
            return crow::response(400, "Failed to generate puzzle");
        }

        crow::json::wvalue resp;
        for (int i = 0; i < size; i++)
            for (int j = 0; j < size; j++)
                resp["grid"][i][j] = sudoku.grid[i][j];

        return crow::response(resp);
    });
    
    app.port(18080).multithreaded().run();
}