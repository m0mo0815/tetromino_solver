#include <iostream>
#include <algorithm>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>
#include <queue>
#include <bitset>

// Constants for grid and tetromino sizes
const int MAX_ROWS = 100;
const int MAX_COLS = 100;
const int NUM_TETROMINOES = 19;
const int TETROMINO_SIZE = 4;

// Bitwise representation of tetrominoes (16-bit integers)
uint16_t TETROMINOES[NUM_TETROMINOES] = {
    0b1111000000000000, // I1
    0b1000100010001000, // I2
    0b1100110000000000, // O
    0b0100111000000000, // T1
    0b1000110001000000, // T2
    0b1110010000000000, // T3
    0b0100110001000000, // T4
    0b0110011000000000, // S1
    0b1000110010000000, // S2
    0b1100011000000000, // Z1
    0b0100110010000000, // Z2
    0b1000111000000000, // J1
    0b1100100010000000, // J2
    0b1110001000000000, // J3
    0b0100010011000000, // J4
    0b0010011100000000, // L1
    0b1000100011000000, // L2
    0b1111000000000000, // L3
    0b1100100100000000  // L4
};

// Function to create an empty grid
void create_grid(int* grid, int rows, int cols) {
    std::fill(grid, grid + rows * cols, 0);
}

// Function to export the grid to a file
void export_grid(int* grid, int rows, int cols, const std::string& filename, bool append) {
    std::ios_base::openmode mode = append ? std::ios::app : std::ios::out;
    std::ofstream file(filename, mode);
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            file << grid[i * cols + j] << " ";
        }
        file << "\n";
    }
    file << "\n";
    file.close();
}

bool can_place_tetromino(int* grid, uint16_t tetromino, int top_left_row, int top_left_col, int rows, int cols) {
    int base_grid_index = top_left_row * cols + top_left_col;
    
    for (int i = 0, row_offset = 0; i < TETROMINO_SIZE; ++i, row_offset += TETROMINO_SIZE) {
        int grid_row_offset = i * cols;
        for (int j = 0; j < TETROMINO_SIZE; ++j) {
            int bit_index = 15 - (row_offset + j);
            if (tetromino & (1 << bit_index)) {  // Bitwise check
                int grid_index = base_grid_index + grid_row_offset + j;
                if (top_left_row + i >= rows || top_left_col + j >= cols || grid[grid_index] != 0) {
                    return false;
                }
            }
        }
    }
    return true;
}

// Function to place a tetromino on the grid
void place_tetromino(int* grid, uint16_t tetromino, int top_left_row, int top_left_col, int unique_id, int cols) {
    int base_grid_index = top_left_row * cols + top_left_col;

    for (int i = 0, row_offset = 0; i < TETROMINO_SIZE; ++i, row_offset += TETROMINO_SIZE) {
        int grid_row_offset = i * cols;
        for (int j = 0; j < TETROMINO_SIZE; ++j) {
            int bit_index = 15 - (row_offset + j);
            if (tetromino & (1 << bit_index)) {  // Bitwise check
                grid[base_grid_index + grid_row_offset + j] = unique_id;
            }
        }
    }
}

// Function to remove a tetromino from the grid
void remove_tetromino(int* grid, uint16_t tetromino, int top_left_row, int top_left_col, int unique_id, int cols) {
    int base_grid_index = top_left_row * cols + top_left_col;

    for (int i = 0, row_offset = 0; i < TETROMINO_SIZE; ++i, row_offset += TETROMINO_SIZE) {
        int grid_row_offset = i * cols;
        for (int j = 0; j < TETROMINO_SIZE; ++j) {
            int bit_index = 15 - (row_offset + j);
            if (tetromino & (1 << bit_index)) {  // Bitwise check
                int grid_index = base_grid_index + grid_row_offset + j;
                if (grid[grid_index] == unique_id) {
                    grid[grid_index] = 0;
                }
            }
        }
    }
}

// Function to calculate the heuristic score for a given tetromino
double calculate_heuristic_score(uint16_t tetromino, int usage_count) {
    int cells_covered = std::bitset<16>(tetromino).count();  // Number of bits set to 1
    int complexity = 0;

    for (int i = 0, row_offset = 0; i < TETROMINO_SIZE - 1; ++i, row_offset += TETROMINO_SIZE) {
        for (int j = 0; j < TETROMINO_SIZE - 1; ++j) {
            int bit_pos = 15 - (row_offset + j);
            bool cur = tetromino & (1 << bit_pos);
            bool right = tetromino & (1 << (bit_pos - 1));
            bool down = tetromino & (1 << (bit_pos - TETROMINO_SIZE));
            complexity += cur && (!right || !down);
        }
    }

    double size_factor = static_cast<double>(cells_covered);
    double complexity_factor = static_cast<double>(complexity);
    double usage_penalty = std::log(usage_count + 1);  // Logarithmic penalty for overuse

    return size_factor + complexity_factor - usage_penalty;
}

// Function to check if there are isolated cells
bool has_isolated_cells(int* grid, int rows, int cols) {
    std::vector<bool> visited(rows * cols, false);

    auto is_within_bounds = [&](int r, int c) {
        return r >= 0 && r < rows && c >= 0 && c < cols;
    };

auto bfs = [&](int r, int c) {
    std::queue<std::pair<int, int>> q;
    q.push({r, c});
    visited[r * cols + c] = true;
    int cell_count = 0;

    while (!q.empty()) {
        auto [cr, cc] = q.front();
        q.pop();
        cell_count++;

        // Unrolled loop for four directions: up, down, left, right
        int nr, nc;

        // Up
        nr = cr - 1;
        nc = cc;
        if (is_within_bounds(nr, nc) && grid[nr * cols + nc] == 0 && !visited[nr * cols + nc]) {
            visited[nr * cols + nc] = true;
            q.push({nr, nc});
        }

        // Down
        nr = cr + 1;
        nc = cc;
        if (is_within_bounds(nr, nc) && grid[nr * cols + nc] == 0 && !visited[nr * cols + nc]) {
            visited[nr * cols + nc] = true;
            q.push({nr, nc});
        }

        // Left
        nr = cr;
        nc = cc - 1;
        if (is_within_bounds(nr, nc) && grid[nr * cols + nc] == 0 && !visited[nr * cols + nc]) {
            visited[nr * cols + nc] = true;
            q.push({nr, nc});
        }

        // Right
        nr = cr;
        nc = cc + 1;
        if (is_within_bounds(nr, nc) && grid[nr * cols + nc] == 0 && !visited[nr * cols + nc]) {
            visited[nr * cols + nc] = true;
            q.push({nr, nc});
        }
    }
    return cell_count;
};


    int total_empty_cells = 0;
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (grid[i * cols + j] == 0 && !visited[i * cols + j]) {
                int cell_count = bfs(i, j);
                total_empty_cells += cell_count;

                if (cell_count % 4 != 0) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Function to fill the grid with tetrominoes
bool fill_grid(int* grid, int tetromino_usage[NUM_TETROMINOES], int current_id, std::chrono::steady_clock::time_point& last_export_time, int export_interval, int rows, int cols, const std::string& base_filename) {
    int row = -1, col = -1;

    // Find the first empty cell for this recursive level
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (grid[i * cols + j] == 0) {
                row = i;
                col = j;
                break;
            }
        }
        if (row != -1) break;
    }

    if (row == -1 && col == -1) return true;

    std::vector<std::pair<int, double>> heuristics;
    for (int i = 0; i < NUM_TETROMINOES; ++i) {
        double score = calculate_heuristic_score(TETROMINOES[i], tetromino_usage[i]);
        heuristics.push_back({i, score});
    }

    std::sort(heuristics.begin(), heuristics.end(), [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second > b.second;
    });

    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::minutes>(now - last_export_time).count() >= export_interval) {
        export_grid(grid, rows, cols, base_filename + "_grid_output_intermediate.txt", true);
        last_export_time = now;
    }

    for (const auto& h : heuristics) {
        int t = h.first;
        if (can_place_tetromino(grid, TETROMINOES[t], row, col, rows, cols)) {
            place_tetromino(grid, TETROMINOES[t], row, col, current_id, cols);
            tetromino_usage[t]++;

            if (!has_isolated_cells(grid, rows, cols)) {
                if (fill_grid(grid, tetromino_usage, current_id + 1, last_export_time, export_interval, rows, cols, base_filename)) {
                    return true;
                }
            }

            remove_tetromino(grid, TETROMINOES[t], row, col, current_id, cols);
            tetromino_usage[t]--;
        }
    }

    return false;
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

    int* grid = new int[rows * cols]();
    int tetromino_usage[NUM_TETROMINOES] = {0};
    int export_interval = 1;

    create_grid(grid, rows, cols);
    std::srand(std::time(0));

    auto last_export_time = std::chrono::steady_clock::now();
    std::string base_filename = "output/" + unique_id + "_" + std::to_string(rows) + "x" + std::to_string(cols);

    if (fill_grid(grid, tetromino_usage, 1, last_export_time, export_interval, rows, cols, base_filename)) {
        export_grid(grid, rows, cols, base_filename + "_grid_output.txt", false);
        std::cout << "Grid solved and exported to " << base_filename + "_grid_output.txt\n";
    } else {
        std::cout << "Could not fill the grid completely.\n";
    }

    delete[] grid;
    return 0;
}
