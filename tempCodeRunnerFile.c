#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

// Define the size of the maze, can be changed
#define MAZE_WIDTH 10
#define MAZE_HEIGHT 10
#define CELL_SIZE 20 // Cell size 20x20 pixels in GTK window

// Paths: North, South, East, and West
// Using hexadecimal as bit flag
// Cause it's memory efficient as it can store multiple directions in a single int using bitwise operations
enum {
    CELL_PATH_N = 0x01, // North (up) = 00001 (binary)
    CELL_PATH_S = 0x02, // South (down) = 00010
    CELL_PATH_E = 0x04, // East (left) = 00100
    CELL_PATH_W = 0x08, // West (right) = 01000
    CELL_VISITED = 0x10, // Visited cells = 10000
};

// Coordinate structure to store x and y
typedef struct coords {
    int x, y;
} Point;

// Array to store cells' current state (i.e., visited?)
int maze[MAZE_WIDTH * MAZE_HEIGHT];
// Stack for backtracking
Point stack[MAZE_WIDTH * MAZE_HEIGHT];
// Track number of elements in stack
int stack_size = 0;
// Track number of visited cells
int visited_cell = 0;

// Check if stack is empty, then return 0
bool isEmpty() {
    return stack_size == 0;
}

// PUSH function, return updated count
void push(Point p) {
    stack[stack_size++] = p;
}

// POP function, return updated count
Point pop() {
    return stack[--stack_size];
}

// Function that converts 2D array to 1D array index.
// For easier access to 2D grid using 1D array
int get_index(int x, int y) {
    return y * MAZE_WIDTH + x;
}

// Maze generating algorithm: DFS, recursive backtracking
void generate_maze() {
    memset(maze, 0, sizeof(maze));
    srand(time(NULL));

    // Starting position
    Point start = { rand() % MAZE_WIDTH, rand() % MAZE_HEIGHT }; // Randomly pick an x and y coordinate
    push(start); // Push starting cell onto stack
    maze[get_index(start.x, start.y)] |= CELL_VISITED; // Mark starting cell as visited
    visited_cell = 1;

    // While there are unvisited cells
    while (visited_cell < MAZE_WIDTH * MAZE_HEIGHT) {
        Point top = stack[stack_size - 1]; // Get the top cell from the stack
        int x = top.x;
        int y = top.y;

        // Array to store valid neighbors
        int neighbors[4] = {-1, -1, -1, -1}; // Initialize to invalid values
        int count = 0;

        // Check North neighbor
        if (y > 0 && !(maze[get_index(x, y - 1)] & CELL_VISITED))
            neighbors[count++] = 0; // North

        // Check East neighbor
        if (x < MAZE_WIDTH - 1 && !(maze[get_index(x + 1, y)] & CELL_VISITED))
            neighbors[count++] = 1; // East

        // Check South neighbor
        if (y < MAZE_HEIGHT - 1 && !(maze[get_index(x, y + 1)] & CELL_VISITED))
            neighbors[count++] = 2; // South

        // Check West neighbor
        if (x > 0 && !(maze[get_index(x - 1, y)] & CELL_VISITED))
            neighbors[count++] = 3; // West

        // If there are valid neighbors, choose one randomly
        if (count > 0) {
            int dir = neighbors[rand() % count]; // Randomly select a direction

            switch (dir) {
                case 0: // North
                    maze[get_index(x, y - 1)] |= CELL_VISITED | CELL_PATH_S;
                    maze[get_index(x, y)] |= CELL_PATH_N;
                    push((Point){x, y - 1});
                    break;

                case 1: // East
                    maze[get_index(x + 1, y)] |= CELL_VISITED | CELL_PATH_W;
                    maze[get_index(x, y)] |= CELL_PATH_E;
                    push((Point){x + 1, y});
                    break;

                case 2: // South
                    maze[get_index(x, y + 1)] |= CELL_VISITED | CELL_PATH_N;
                    maze[get_index(x, y)] |= CELL_PATH_S;
                    push((Point){x, y + 1});
                    break;

                case 3: // West
                    maze[get_index(x - 1, y)] |= CELL_VISITED | CELL_PATH_E;
                    maze[get_index(x, y)] |= CELL_PATH_W;
                    push((Point){x - 1, y});
                    break;
            }
            visited_cell++; // Increment visited cell count
        } else {
            pop(); // Backtrack if no valid neighbors
        }
    }
}

void convert_maze(int converted[MAZE_HEIGHT][MAZE_WIDTH], int initial[MAZE_HEIGHT * MAZE_WIDTH]) {
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            converted[i][j] = initial[get_index(j, i)]; // Corrected indexing: (j, i) instead of (i, j)
        }
    }
}

void print_converted(int convertedmaze[MAZE_HEIGHT][MAZE_WIDTH]) {
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            printf(" %d ", convertedmaze[i][j]);
        }
        printf("\n");
    }
}

int main() {
    generate_maze(); // Call the generate_maze function to generate the maze

    int convertedmaze[MAZE_HEIGHT][MAZE_WIDTH];
    convert_maze(convertedmaze, maze); // Convert the 1D maze array to a 2D array
    print_converted(convertedmaze); // Print the converted maze

    return 0;
}