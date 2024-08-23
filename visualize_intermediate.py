import streamlit as st
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from pathlib import Path

# Load all grid data from the file, handling multiple grids separated by empty lines
def load_all_grids(filename):
    with open(filename, 'r') as file:
        raw_grids = file.read().strip().split('\n\n')  # Split grids by empty lines
        grids = [np.array([list(map(int, line.split())) for line in grid.split('\n')]) for grid in raw_grids]
    return grids

# Function to check if a grid cell is connected to its neighbors (for black-and-white mode)
def is_connected(grid, row, col):
    connected = []
    if row > 0 and grid[row, col] == grid[row - 1, col]:
        connected.append('top')
    if row < grid.shape[0] - 1 and grid[row, col] == grid[row + 1, col]:
        connected.append('bottom')
    if col > 0 and grid[row, col] == grid[row, col - 1]:
        connected.append('left')
    if col < grid.shape[1] - 1 and grid[row, col] == grid[row, col + 1]:
        connected.append('right')
    return connected

# Visualize a specific grid and save the image
def visualize_grid(grid, save_path):
    fig, ax = plt.subplots(figsize=(8, 8))  # Adjust figure size as needed
    for row in range(grid.shape[0]):
        for col in range(grid.shape[1]):
            if grid[row, col] != 0:
                connected = is_connected(grid, row, col)
                rect = patches.Rectangle((col, row), 1, 1, facecolor='#333', edgecolor='none')
                ax.add_patch(rect)

                if 'top' not in connected:
                    ax.plot([col, col + 1], [row, row], color='white', lw=1)
                if 'bottom' not in connected:
                    ax.plot([col, col + 1], [row + 1, row + 1], color='white', lw=1)
                if 'left' not in connected:
                    ax.plot([col, col], [row, row + 1], color='white', lw=1)
                if 'right' not in connected:
                    ax.plot([col + 1, col + 1], [row, row + 1], color='white', lw=1)

    ax.set_aspect('equal')
    ax.set_xlim(0, grid.shape[1])
    ax.set_ylim(0, grid.shape[0])
    ax.invert_yaxis()
    ax.set_xticks([])
    ax.set_yticks([])
    ax.grid(False)
    for spine in ax.spines.values():
        spine.set_visible(False)
    
    plt.savefig(save_path)
    plt.close(fig)

# Streamlit application
def main():
    st.title("Grid Evolution Visualizer")
    
    # Specify the filename directly
    filename = "output/2d4ccd7b5d80508140af1cb4c13f145c2ab290b9_80x80_grid_output_intermediate.txt"
    
    # Load all grids data
    grids = load_all_grids(filename)
    
    # Specify a directory for storing the images
    img_dir = Path("output/streamlit_pre_generated_images")
    img_dir.mkdir(exist_ok=True)  # Create the directory if it doesn't exist
    
    # Pre-generate images and save paths
    image_paths = []
    for i, grid in enumerate(grids):
        img_path = img_dir / f"grid_step_{i}.png"
        if not img_path.exists():  # Only generate if not already present
            visualize_grid(grid, img_path)
        image_paths.append(img_path)
    
    # Initialize session state if not already set
    if 'current_step' not in st.session_state:
        st.session_state.current_step = 0

    step_increment = st.number_input('Step Increment', min_value=1, value=1)

    col1, col2 = st.columns(2)
    with col1:
        if st.button('Previous'):
            st.session_state.current_step = max(st.session_state.current_step - step_increment, 0)
    with col2:
        if st.button('Next'):
            st.session_state.current_step = min(st.session_state.current_step + step_increment, len(grids) - 1)
    
    # Display the pre-generated image
    st.image(str(image_paths[st.session_state.current_step]))

if __name__ == "__main__":
    main()
