#include <iostream>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>

// Constants for grid and tetromino sizes
const int MAX_ROWS = 100;
const int MAX_COLS = 100;
const int NUM_TETROMINOES = 19;

int TETROMINOES[NUM_TETROMINOES][4][4] = {
    {{1, 1, 1, 1}},                          // I1
    {{1}, {1}, {1}, {1}},                    // I2
    {{1, 1}, {1, 1}},                        // O
    {{0, 1, 0}, {1, 1, 1}},                  // T1
    {{1, 0}, {1, 1}, {1, 0}},                // T2
    {{1, 1, 1}, {0, 1, 0}},                  // T3
    {{0, 1}, {1, 1}, {0, 1}},                // T4
    {{0, 1, 1}, {1, 1, 0}},                  // S1
    {{1, 0}, {1, 1}, {0, 1}},                // S2
    {{1, 1, 0}, {0, 1, 1}},                  // Z1
    {{0, 1}, {1, 1}, {1, 0}},                // Z2
    {{1, 0, 0}, {1, 1, 1}},                  // J1
    {{1, 1}, {1, 0}, {1, 0}},                // J2
    {{1, 1, 1}, {0, 0, 1}},                  // J3
    {{0, 1}, {0, 1}, {1, 1}},                // J4
    {{0, 0, 1}, {1, 1, 1}},                  // L1
    {{1, 0}, {1, 0}, {1, 1}},                // L2
    {{1, 1, 1}, {1, 0, 0}},                  // L3
    {{1, 1}, {0, 1}, {0, 1}}                 // L4
};

// Function to calculate the heuristic score for a given tetromino
double calculate_heuristic_score(const int tetromino[4][4], int usage_count) {
    int cells_covered = 0;
    int complexity = 0;

    // Calculate cells covered and complexity (number of turns in the shape)
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (tetromino[i][j] == 1) {
                cells_covered++;
                // Count complexity based on the number of turns in the shape
                if ((i > 0 && tetromino[i-1][j] == 0) || (j > 0 && tetromino[i][j-1] == 0)) {
                    complexity++;
                }
            }
        }
    }

    // Heuristic score is a combination of size, complexity, and usage penalty
    double size_factor = static_cast<double>(cells_covered);
    double complexity_factor = static_cast<double>(complexity);
    double usage_penalty = std::log(usage_count + 1); // Logarithmic penalty for overuse

    return size_factor + complexity_factor - usage_penalty;
}

// Function to create an empty grid
void create_grid(int** grid, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            grid[i][j] = 0;
        }
    }
}

// Function to export the grid to a file
void export_grid(int** grid, int rows, int cols, const std::string& filename, bool append) {
    std::ios_base::openmode mode = append ? std::ios::app : std::ios::out;
    std::ofstream file(filename, mode);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            file << grid[i][j] << " ";
        }
        file << "\n";
    }
    file << "\n";
    file.close();
}

// Function to check if a tetromino can be placed on the grid
bool can_place_tetromino(int** grid, const int tetromino[4][4], int top_left_row, int top_left_col, int rows, int cols) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (tetromino[i][j] == 1) {
                int grid_row = top_left_row + i;
                int grid_col = top_left_col + j;
                if (grid_row >= rows || grid_col >= cols || grid[grid_row][grid_col] != 0) {
                    return false;
                }
            }
        }
    }
    return true;
}

// Function to place a tetromino on the grid
void place_tetromino(int** grid, const int tetromino[4][4], int top_left_row, int top_left_col, int unique_id) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (tetromino[i][j] == 1) {
                grid[top_left_row + i][top_left_col + j] = unique_id; // Use unique_id to mark placement
            }
        }
    }
}

// Function to remove a tetromino from the grid
void remove_tetromino(int** grid, const int tetromino[4][4], int top_left_row, int top_left_col, int unique_id) {
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            if (tetromino[i][j] == 1 && grid[top_left_row + i][top_left_col + j] == unique_id) {
                grid[top_left_row + i][top_left_col + j] = 0; // Only remove cells marked with the unique_id
            }
        }
    }
}

bool fill_grid(int** grid, int tetromino_usage[NUM_TETROMINOES], int current_id, std::chrono::steady_clock::time_point& last_export_time, int export_interval, int rows, int cols, const std::string& base_filename) {
    int row = -1, col = -1;

    // Find the first empty cell for this recursive level
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (grid[i][j] == 0) {
                row = i;
                col = j;
                break;
            }
        }
        if (row != -1) break;
    }

    // If no empty cell was found, the grid is fully filled
    if (row == -1 && col == -1) {
        return true;
    }

    // Calculate heuristic scores for each tetromino
    std::vector<std::pair<int, double>> heuristics;
    for (int i = 0; i < NUM_TETROMINOES; ++i) {
        double score = calculate_heuristic_score(TETROMINOES[i], tetromino_usage[i]);
        heuristics.push_back({i, score});
    }

    // Sort tetromino indices based on heuristic scores (descending)
    std::sort(heuristics.begin(), heuristics.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second > b.second;
    });

    // Check if it's time to export the current grid state
    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::minutes>(now - last_export_time).count() >= export_interval) {
        export_grid(grid, rows, cols, base_filename + "_grid_output_intermediate.txt", true); // Append to the intermediate file
        last_export_time = now;
    }

    // Try placing each tetromino in the sorted order
    for (const auto& h : heuristics) {
        int t = h.first;
        if (can_place_tetromino(grid, TETROMINOES[t], row, col, rows, cols)) {
            place_tetromino(grid, TETROMINOES[t], row, col, current_id); // Provide unique_id
            tetromino_usage[t]++;

            // Recurse to continue placing tetrominoes
            if (fill_grid(grid, tetromino_usage, current_id + 1, last_export_time, export_interval, rows, cols, base_filename)) {
                return true;  // Successfully filled the grid
            }

            // Backtrack: If further placement fails, remove the tetromino
            remove_tetromino(grid, TETROMINOES[t], row, col, current_id); // Provide unique_id
            tetromino_usage[t]--;
        }
    }

    return false; // Backtrack if no placement was successful
}

// Main function
int main(int argc, char** argv) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <rows> <cols> <unique_id>\n";
        return 1;
    }

    int rows = std::stoi(argv[1]);
    int cols = std::stoi(argv[2]);
    std::string unique_id = argv[3];
   
    int** grid = new int*[rows];
        for (int i = 0; i < rows; ++i) {
            grid[i] = new int[cols]();
        }

    int tetromino_usage[NUM_TETROMINOES] = {0};
    int export_interval = 1; // Export interval in minutes

    create_grid(grid, rows, cols);
    std::srand(std::time(0));

    // Initialize the time point for the last export
    auto last_export_time = std::chrono::steady_clock::now();

    // Create the base filename
    std::string base_filename = "output/" + unique_id + "_" + std::to_string(rows) + "x" + std::to_string(cols);

    if (fill_grid(grid, tetromino_usage, 1, last_export_time, export_interval, rows, cols, base_filename)) {
        export_grid(grid, rows, cols, base_filename + "_grid_output.txt", false); // Overwrite the final grid file
        std::cout << "Grid solved and exported to " << base_filename + "_grid_output.txt\n";
    } else {
        std::cout << "Could not fill the grid completely.\n";
    }

    for (int i = 0; i < rows; ++i) {
        delete[] grid[i];
    }
    delete[] grid;

    return 0;
}
