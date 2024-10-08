# Compiler and flags
CXX = g++-14
CXXFLAGS = -std=c++17 -O3 -march=armv8-a

# Source files and executable
SRC = tetromino_solver.cpp
EXE = tetromino_solver

# Output directory
OUTPUT_DIR = output

# Generate a unique ID based on the hash of the source file
UNIQUE_ID = $(shell shasum $(SRC) | awk '{print $$1}')

# Grid sizes to run
GRID_SIZES = 50x50

# Python virtual environment called playground based on python 3.9.18 - see requirements.txt
VENV_PYTHON = ~/.pyenv/versions/playground/bin/python3

.PHONY: all run clean visualize full_run

all: $(EXE)

$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(EXE) $(SRC)

run: $(EXE)
	@for size in $(GRID_SIZES); do \
		rows=$$(echo $$size | cut -d'x' -f1); \
		cols=$$(echo $$size | cut -d'x' -f2); \
		echo "Running with $$rows rows and $$cols cols..."; \
		time ./$(EXE) $$rows $$cols $(UNIQUE_ID) > output/log_output.txt; \
	done

visualize: run
	$(VENV_PYTHON) visualize_grids.py $(UNIQUE_ID)

full_run: all run visualize
	@echo "All steps completed."