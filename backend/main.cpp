#include "crow_all.h"
#include "6x6.cpp"

struct CORSMiddleware {
    struct context {};
    void before_handle(crow::request&, crow::response& res, context&) {}
    void after_handle(crow::request&, crow::response& res, context&) {
        res.add_header("Access-Control-Allow-Origin", "*");
    }
};

int main() {
    crow::App<CORSMiddleware> app;

    CROW_ROUTE(app, "/")
    ([](){
        crow::response res("Sudoku server is running!");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    CROW_ROUTE(app, "/new").methods("OPTIONS"_method)
    ([](const crow::request&){
        crow::response res(204);
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type");
        return res;
    });

    CROW_ROUTE(app, "/new").methods("GET"_method)
    ([](const crow::request& req){
       auto size = std::stoi(req.url_params.get("size") ? req.url_params.get("size") : "4");
       auto diffStr = std::string(req.url_params.get("difficulty") ? req.url_params.get("difficulty") : "medium");
       auto showSol = std::string(req.url_params.get("showSolution") ? req.url_params.get("showSolution") : "false") == "true";

       int boxH = (size == 4) ? 2 : 3;
       int boxW = (size == 4) ? 2 : 2;

       Difficulty diff = MEDIUM;
       if (diffStr == "easy") diff = EASY;
       if (diffStr == "hard") diff = HARD;

       Sudoku6 sudoku(size, boxH, boxW);
       if (!sudoku.generatePuzzle(diff)) {
        crow::response res(400, "Failed to generate puzzle.");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
       }

       crow::json::wvalue resp;
       for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                resp["puzzle"][i][j] = sudoku.grid[i][j];
            }
       }
       
       if (showSol) {
            for (int i = 0; i < size; i++) {
                for (int j = 0; j < size; j++) {
                    resp["solution"][i][j] = sudoku.solution[i][j];
                }
            }
       }

       resp["size"] = size;
       resp["difficulty"] = diffStr;
       crow::response res(resp);
       return res;
    });
    app.port(18080).multithreaded().run();
}