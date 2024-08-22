- improve output quality of PNG
- requirements.txt

- cache heuristics
- paralellisation OpenMP
    - tetromino placements
- profiling

A) Technical Improvements (Ordered by Likely Impact)

    Early Exit Conditions:
        Medium Impact: Reduces unnecessary calculations when it’s already clear that a tetromino can’t be placed.
        Reason: Exiting early saves time during tight loops.

B) Algorithmic Improvements (Ordered by Likely Impact)


    Symmetry Reduction:
        Medium to High Impact: Reduces the number of equivalent configurations that need to be explored.
        Reason: Prevents redundant work by leveraging symmetry in the grid and problem space.

    Multi-Directional Search:
        Medium Impact: Helps avoid poor initial decisions that can lead to failure deep in the recursion.
        Reason: Starting from multiple points reduces dependency on the first few placements.

    Dynamic Programming Memoization:
        Medium Impact: Effective if there are many overlapping subproblems.
        Reason: Reuses solutions to subproblems instead of recalculating them.

    Iterative Deepening:
        Low to Medium Impact: Can avoid deep, fruitless searches, but may not be as effective in all scenarios.
        Reason: By finding simpler solutions first, you avoid getting stuck in complex, unlikely paths.

    Guided Randomization:
        Low Impact: Useful if the search space is vast and heuristic ordering is difficult to define.
        Reason: Introduces a probabilistic element to escape local minima, but may require significant tuning.

1. Reduce Function Call Overheads:

Inline small functions like can_place_tetromino, place_tetromino, and remove_tetromino. This eliminates the overhead associated with function calls, especially for frequently called functions.
2. Profile-Driven Optimization:

Use profiling tools (like gprof or perf) to identify bottlenecks in the code. You can then focus optimizations on the parts of the code that consume the most time.
3. Parallelization:

Introduce multithreading to explore different grid-filling possibilities in parallel. This can be done using OpenMP or C++ standard threads.
4. Precompute and Cache Results:

For certain calculations that are repeatedly performed, precompute results and store them in a lookup table. For example, you could cache the results of heuristic calculations for tetrominoes in different configurations.
5. Advanced Heuristics:

Further refine the heuristics used to score tetromino placement. More sophisticated heuristics that take into account factors like future placement options, density of occupied cells, and connectivity could guide the search more effectively.
6. Optimized Backtracking:

Implement advanced backtracking techniques like constraint propagation or conflict-driven clause learning (CDCL) to avoid exploring unproductive branches of the search tree.
7. Grid Splitting (Divide and Conquer):

Consider breaking down the grid into smaller sections, solving each section independently, and then merging the results. This approach works well when combined with multi-threading.

## Done

        Heuristic Ordering:
        High Impact: Proper heuristics can greatly reduce the depth and breadth of the search tree.
        Reason: By prioritizing tricky placements, you reduce the likelihood of creating unfillable gaps early.

        Backtracking Pruning:
        High Impact: Aggressive pruning can cut off large parts of the search tree early.
        Reason: Prevents wasting time on impossible-to-complete configurations.

        Bitwise Representation:
        High Impact: This can drastically reduce the memory footprint and allow very fast operations using bitwise logic.
        Reason: Reduces the complexity of operations like checking and placing tetrominoes from O(1) per cell to O(1) per bit operation.

        Memory Optimization:
        Medium Impact: Improves cache usage and reduces memory access time.
        Reason: Linear arrays improve spatial locality and cache performance.