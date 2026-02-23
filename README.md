# Project Daedalus by Team WCKD: Algorithmic Maze Generation & Solver

## 📌 Project Overview
Project Daedalus is a C-based engineering application that demonstrates the intersection of graph theory and automated pathfinding. The program dynamically generates a complex, randomized square maze using recursive backtracking and identifies the most efficient escape route using a customized implementation of **Dijkstra’s Algorithm**.

This project was developed as a final technical challenge for **MREN 178** at Queen's University to showcase proficiency in low-level memory management and algorithmic logic.



## 🚀 Key Features
* **Dynamic Generation:** Uses a Depth-First Search (DFS) algorithm to ensure every maze is unique odd x odd square maze but solvable, with an exit at every sides of the maze.
* **Optimized Pathfinding:** Implements Dijkstra’s Algorithm to evaluate four different exits simultaneously and highlight the  mathematically shortest path.
* **Memory Efficiency:** Utilizes bitwise operations and hexadecimal bit flags to store multi-state cell data (Path, Visited, Exit, Solution) within a single integer.
* **Interactive UI:** A terminal-based visualization system that renders the maze in real-time and reveals the solution upon user command.

## 🛠 Technical Stack
* **Language:** C
* **Concepts:** Graph Theory, Recursion, Bitwise Manipulation, Dynamic Memory Allocation (`malloc`/`free`).
* **Algorithms:** Dijkstra’s Algorithm, Randomized Depth-First Search (DFS).

## 🧠 How It Works

### 1. Generation (Recursive Backtracking)
The maze starts from a central 3x3 "origin" box, and ask for the users' input of how big they want their maze to be(it has to be an odd number over 7). The algorithm then moves to a random unvisited neighbor, carving a path and pushing the location onto a stack. When it hits a dead end, it pops from the stack to backtrack until it finds a cell with unvisited neighbors, ensuring 100% coverage of the grid.



### 2. Solving (Dijkstra's Algorithm)
The solver treats the maze as a weighted graph where each cell is a node and each path is an edge with a weight of 1. 
* It calculates the cumulative distance from the center to all reachable nodes.
* It tracks "previous" pointers for every cell to reconstruct the path.
* It compares the distance to all four exits and selects the one with the lowest total cost.

## 📂 File Structure
* `main.c`: Contains the core logic for generation, solving, and memory management.
* `structs`: Custom definitions for `Point` (coordinate tracking) and `Cell` (graph node data).
