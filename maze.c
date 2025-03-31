#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

/*
MREN 178 FINAL PROJECT - Project Daedalus
by Team WCKD
- Lucas Nguyen (20444343)
- Andrew Thomas (20448068)
- Ethan Wu (20453336)
- Eamonn Wong (20445086)

March 31, 2025

This program generates a square maze of specified dimension with four exits that begins in the centre. It displays this maze for the user to attempt to solve
and then uses a variation of Dijkstra's algorithm to find the shorest path out of the maze (could be any exit) and displays this solution.

Attributions: a YouTube video by javidx9 was used to build familiarity with recursive backtracking in maze generation. In class notes were referenced, particularly
regards to the key features of Dijkstra's algorithm.
*/

//Define the size of the maze, can be changed
#define MAZE_WIDTH 31 //use an odd number
#define MAZE_HEIGHT 31 //use an odd number (note, must be square)
#define INF 9999

//Paths: North, South, East, and West
//Using hexidecimal as bit flag
//memory efficient, as it can store multiple direction in a single int using bitwise operations
enum{
    CELL_PATH_N = 0x01, //North(up) = 1
    CELL_PATH_S = 0x02, //South(down) = 2
    CELL_PATH_E = 0x04, //East(left) = 4
    CELL_PATH_W = 0x08, //West(right) = 8
    CELL_VISITED = 0x10, //Visisted cells = 16
    CELL_EXIT = 0x20, //cell is one of the exits = 32
    CELL_COLOUR = 0x40, //cell is in an exit path = 64
};

//Coordinate structure to store x and y
typedef struct coords{
    int x, y;
}Point;

//structure for storing information on each cell while solving
typedef struct cell{
    int x, y;
    int cellVal; //from generation array, will be used to determine open paths
    struct cell* upNeigh; //north neighbour
    struct cell* downNeigh; //south neighbour
    struct cell* leftNeigh; //west neighbour
    struct cell* rightNeigh; //east neighbour
    int distance; //stores distance from centre
    bool visited; //keeps track of if it has been visited
    struct cell* previous; //stores next cell on shortest path to centre
}Cell;

//array to store cells current state(i.e visited?)
int maze[MAZE_HEIGHT * MAZE_WIDTH];
//3D array to contain all 4 paths to exits
int paths[4][2][MAZE_HEIGHT * MAZE_WIDTH] = {0};
//2D array of exit locations
Cell* exitLocations[4];
//stack for gen back tracking
Point stack[MAZE_HEIGHT * MAZE_WIDTH];
//stack for solving
int distances[MAZE_HEIGHT][MAZE_WIDTH];
//track number of elements in stack
int stack_size = -1;
//track number of visited cells
int visited_cell = 0;
//keep track of closest exit
int shortestE = 0;
//number of locatable exits
int exitsFound = 0;
//2d array of cell structs for solving
Cell* graph[MAZE_HEIGHT][MAZE_WIDTH] = {NULL}; //y val then x val
//declare visual maze of be displayed
char visual_maze[(MAZE_HEIGHT*2)+1][(MAZE_WIDTH*2)+1];

//Check if stack is empty, then return 0, for generation
bool isEmpty(){
   return stack_size == 0;
}

//Push function, return updated count, for generation
void pushP(Point p){
    stack[stack_size++] = p; //store the point struct at the top index then increase that index
}

//Pop Function, return updated count, for generation
Point popP(){
    return stack[--stack_size]; //decrement the top index and return that point struct
}


//Function that converts 2D array to 1D array index.
//For easier access to 2D grid using 1D array
int get_index(int x, int y){
    return (y * MAZE_WIDTH) + x;
}

//Intialize starting center box, 3x3
void intialize_center_box(){
    //Start top left corner of 3x3 box
    int start_x = MAZE_WIDTH / 2 - 1;
    int start_y = MAZE_HEIGHT / 2 - 1;
    //Need to iterate through 3 rows, 3 columns

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            //get coordinates of the cell in the box
            int x = start_x + j;
            int y = start_y + i;
            int index = get_index(x, y);

            maze[index] |= CELL_VISITED; //set each of the 9 cell ints to visited (bitwise)
            visited_cell++;
        }
    }

    //manually set the open paths for the five cells in the centre cross
    //top of cross
    maze[get_index(start_x + 1, start_y)] |= CELL_PATH_N;
    maze[get_index(start_x + 1, start_y)] |= CELL_PATH_S;
    //bottom of cross
    maze[get_index(start_x + 1, start_y + 2)] |= CELL_PATH_N;
    maze[get_index(start_x + 1, start_y + 2)] |= CELL_PATH_S;
    //left of cross
    maze[get_index(start_x, start_y + 1)] |= CELL_PATH_E;
    maze[get_index(start_x, start_y + 1)] |= CELL_PATH_W;
    //right of cross
    maze[get_index(start_x + 2, start_y + 1)] |= CELL_PATH_E;
    maze[get_index(start_x + 2, start_y + 1)] |= CELL_PATH_W;
    //centre
    maze[get_index(start_x + 1, start_y + 1)] |= CELL_PATH_N;
    maze[get_index(start_x + 1, start_y + 1)] |= CELL_PATH_S;
    maze[get_index(start_x + 1, start_y + 1)] |= CELL_PATH_E;
    maze[get_index(start_x + 1, start_y + 1)] |= CELL_PATH_W;
    
}

//Maze generating algorithm: DFS, recursive backtracking
void generate_maze(){
    memset(maze, 0, sizeof(maze)); //set 1D of cell values to 0
    srand(time(NULL)); //generate random number

    intialize_center_box();

    //starting position
    Point start = {MAZE_WIDTH/2, MAZE_HEIGHT/2}; //generate maze right in the middle
    pushP(start); //starting cell push onto stack
    //Starting cell is marked visited, update visited cell
    maze[get_index(start.x, start.y)] |= CELL_VISITED;
    visited_cell++;

    //while loop to visit all cells on the grid
    while(visited_cell < MAZE_WIDTH * MAZE_HEIGHT) {
        Point top = stack[stack_size -1]; //top = start, where the algorithm going to start running
        int x = top.x;
        int y = top.y;
   
        int neighbors[4]; //neighbors cells around working cell. possible movement directions
        int count = 0;
       
        //Check if each direction has unvisted cell, and keep counts on how many valid neighbors to visit
        //Check if North is unvisited
        if(y > 0 && !(maze[get_index(x, y -1)] & CELL_VISITED))
            neighbors[count++]=0;
        //Check if East is unvisited
        if (x < MAZE_WIDTH - 1 && !(maze[get_index(x + 1, y)] & CELL_VISITED))
            neighbors[count++] = 1;
        //Check if South is unvisited
        if(y < MAZE_HEIGHT - 1 && !(maze[get_index(x, y + 1)] & CELL_VISITED))
            neighbors[count++] = 2;
        //Check if West is unvisited
        if (x > 0 && !(maze[get_index(x - 1, y)] & CELL_VISITED))
            neighbors[count++] = 3;

        //if there is valid unvisited neighbor(s), then randomly go to those valid cells
        if(count > 0){
            int dir = neighbors[rand() % count];
       
        switch(dir){
            case 0: //make north connection
                maze[get_index(x, y - 1)] |= CELL_VISITED | CELL_PATH_S;
                maze[get_index(x, y)] |= CELL_PATH_N;
                pushP((Point){x, y - 1});
                break;
            case 1: //make east connection
                maze[get_index(x + 1, y)] |= CELL_VISITED | CELL_PATH_W;
                maze[get_index(x, y)] |= CELL_PATH_E;
                pushP((Point){x + 1, y});
                break;
            case 2: //make south connection
                maze[get_index(x, y + 1)] |= CELL_VISITED | CELL_PATH_N;
                maze[get_index(x, y)] |= CELL_PATH_S;
                pushP((Point){x, y + 1});
                break;
            case 3: //make west connection
                maze[get_index(x - 1, y)] |= CELL_VISITED | CELL_PATH_E;
                maze[get_index(x, y)] |= CELL_PATH_W;
                pushP((Point){x - 1, y});
                break;
            }
            visited_cell++;
        } else {
            popP();
        }  
    }
}

//function for clearing an index
void unfill(int outputX, int outputY) {
    visual_maze[outputX][outputY] = ' ';
}

void print_maze() {
    //coodinatres on the visual (expanded) array
    int workingX = 0;
    int workingY = 0;
    //fill in entire visual array
    for (int y = 0; y < (MAZE_HEIGHT*2) + 1; y++) { //progess down columns (from one full row to the next)
            for (int x = 0; x < (MAZE_WIDTH*2) + 1; x++) { //go across rows (from one column to the next)
                visual_maze[y][x] = '*';
            }
        }

    //open cores of cells and break down walls
    for (int y = 0; y < MAZE_HEIGHT; y++) { //by row
        for (int x = 0; x < MAZE_WIDTH; x++) { //by column

            printf("%3d ", maze[get_index(x,y)]); //for printing the maze gen number to check 

            //calculate where core falls in the visual array
            workingX = 2*x + 1;
            workingY = 2*y + 1;

            //get the cell number
            int walls = maze[get_index(x, y)];

            //empty the cores unless it is part of the solution path (intially there are no solutions, no pound signs)
            if ((maze[get_index(x, y)] & CELL_COLOUR) == CELL_COLOUR) {
                visual_maze[workingX][workingY] = '#';
            } else {
                unfill(workingX, workingY);
            }

            if ((walls & CELL_PATH_N) == CELL_PATH_N)  //brea north wall
                unfill(workingX, workingY-1);
            
            if ((walls & CELL_PATH_S) == CELL_PATH_S)  //break south wall
                unfill(workingX, workingY+1);
            
            if ((walls & CELL_PATH_E) == CELL_PATH_E)  //break east wall
                unfill(workingX+1, workingY);
            
            if ((walls & CELL_PATH_W) == CELL_PATH_W)  //break west wall
                unfill(workingX-1, workingY);
        }
        printf("\n"); //also for check printing
    } 

    printf("\033[1;32m"); //set colour to green

    //for actually printing to the terminal 
    for (int y = 0; y < (MAZE_HEIGHT*2)+1; y++) { 
        for (int x = 0; x < (MAZE_WIDTH*2)+1; x++) { 
            //check if core is part of solution path or if the index in question lies between two cores that are part of the solution path and print a red pound sign
            if ((visual_maze[x][y] == '#') || ((visual_maze[x][y] == ' ') && (visual_maze[x-1][y] == '#') && (visual_maze[x+1][y] == '#')) || ((visual_maze[x][y] == ' ') && (visual_maze[x][y-1] == '#') && (visual_maze[x][y+1] == '#'))) { //j i flipped down here
                printf("\033[1;31m"); //switch to red
                printf("# ");
                printf("\033[1;32m"); //back to green
            //for printing the center box in white so it stands out
           } else if ((y > ((MAZE_WIDTH*2)+1)/2 - 4) && (y < ((MAZE_WIDTH*2)+1)/2 + 4) && (x > ((MAZE_HEIGHT*2)+1)/2 - 4) && (x < ((MAZE_HEIGHT*2)+1)/2 + 4)) {
                printf("\033[1;37m"); //switch to white
                printf("%c ", visual_maze[x][y]);
                printf("\033[1;32m"); //back to green
            } else {
                printf("%c ", visual_maze[x][y]); //either open space or wall asterix
            }
        }
    printf("\n");
    }
    printf("\033[0m "); //clear colours
}

//Add entrance function to the 3x3 box. Will add entrance on each side of the box, in the middle
void addEntranceToBox(){
    //Top left corner of 3x3
    int start_x = MAZE_WIDTH/2 - 1;
    int start_y = MAZE_HEIGHT/2 - 1;

    //Top side entrance
    maze[get_index(start_x + 1, start_y - 1)] |= CELL_VISITED | CELL_PATH_S;
    unfill((start_y * 2) - 1, (start_x * 2) + 3);

    //Bottom side entrance
    maze[get_index(start_x + 1, start_y + 3)] |= CELL_VISITED | CELL_PATH_N;
    unfill((start_y * 2) + 5, (start_x * 2) + 3);

    // Left side entrance
    maze[get_index(start_x - 1, start_y + 1)] |= CELL_VISITED | CELL_PATH_E;
    unfill((start_y * 2) + 3, (start_x * 2) - 1);

    // Right side entrance
    maze[get_index(start_x + 3, start_y + 1)] |= CELL_VISITED | CELL_PATH_W;
    unfill((start_y * 2) + 3, (start_x * 2) + 7);
}

//Function to add entrance, 4 entrance, 1 on each side of the maze, random position
void addExits(){
    srand(time(NULL)); //ensures different result every run

    //Top side exit, y= 0
    int topExit = (rand() % (MAZE_WIDTH - 2)) + 1; //can't occur in corner
    maze[get_index(topExit, 0)] |= CELL_PATH_N;
    maze[get_index(topExit, 0)] |= CELL_EXIT;
    unfill(0, (topExit * 2) + 1); //cores occupie odd number, so +1 after *2. Repeat for all

    //Bottom side exit, y = -1
    int bottomExit = (rand() % (MAZE_WIDTH - 2)) + 1; //can't occur in corner
    maze[get_index(bottomExit, MAZE_HEIGHT - 1)] |= CELL_PATH_S;
    maze[get_index(bottomExit, MAZE_HEIGHT -1 )] |= CELL_EXIT;
    unfill(MAZE_HEIGHT * 2, (bottomExit * 2) +1);

    //Left side exit, x = 0
    int leftExit = (rand() % (MAZE_HEIGHT - 2)) + 1; //can't occur in corner
    maze[get_index(0, leftExit)] |= CELL_PATH_W;
    maze[get_index(0, leftExit)] |= CELL_EXIT;
    unfill((leftExit * 2) + 1, 0);

    //right side exit, x = -1
    int rightExit = (rand() % (MAZE_HEIGHT - 2)) + 1; //can't occur in corner
    maze[get_index(MAZE_WIDTH - 1, rightExit)] |= CELL_PATH_E;
    maze[get_index(MAZE_WIDTH - 1, rightExit)] |= CELL_EXIT;
    unfill((rightExit * 2) + 1, MAZE_WIDTH * 2);
}

//for creating a new cell
Cell* createCell(int x, int y) {
    Cell* newCell = (Cell*)malloc(sizeof(Cell)); //NULL check done later
    newCell->x = x;
    newCell->y = y;
    newCell->cellVal = maze[get_index(x, y)];
    newCell->upNeigh = NULL;
    newCell->downNeigh = NULL;
    newCell->leftNeigh = NULL;
    newCell->rightNeigh = NULL;
    newCell->distance = INF;
    newCell->visited = false;
    newCell->previous = NULL;
    return newCell;
}

//function will move through the entire maze, allowing for cycles, and keep track of exit coordinates (but won't stop when it finds them)
void generate_graph() {
    printf("started graphing\n");

    //fill the graph array with cell structures at their corresponding coordinates
    for (int y = 0; y < MAZE_HEIGHT; y++) { //down rows
        for (int x = 0; x < MAZE_WIDTH; x++) { //across columns
            Cell* current = createCell(x, y);
            graph[y][x] = current;
        }
    }

    //for making connections between cells that have no walls between them
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            Cell* current = graph[y][x];
            //check if an exit can be noted
            if ((current->cellVal & CELL_EXIT) == CELL_EXIT) {
                exitLocations[exitsFound] = current; //add the cell to the 4 element array of exit cells
                exitsFound++;
                printf("An exit found at y=%d, x=%d!\n", y, x);
            }
            //set neighbour to the adjacent cells in the graph
            if (((current->cellVal & CELL_PATH_N) == CELL_PATH_N) && (y != 0)) { //check that is not trying to break out through one of the exit holes
                current->upNeigh = graph[y-1][x];
            }
            if (((current->cellVal & CELL_PATH_S) == CELL_PATH_S) && (y != (MAZE_HEIGHT - 1))) {
                current->downNeigh = graph[y+1][x];
            }
            if (((current->cellVal & CELL_PATH_W) == CELL_PATH_W) && (x != 0)) {
                current->leftNeigh = graph[y][x-1];
            }
            if (((current->cellVal & CELL_PATH_E) == CELL_PATH_E) && (x != (MAZE_WIDTH - 1))) {
                current->rightNeigh = graph[y][x+1];
            }
        }
    }
    
    printf("graph filled!\n");
}

//check function for visualizing the 2D distances array
void printDists() {
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            printf("%4d ", distances[y][x]);
        }
        printf("\n");
    }
}

//checks if any distances are not infinite (if there are still accessible cells to be explored)
bool allNines() {
    bool foundIssue = false;
    //loop through the 2D distances array
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            if (distances[y][x] != INF) {
                foundIssue = true;
            }
        }
    }
    return foundIssue;
}

//function will run dijkstra's on the graph, adjusting position pointers of each cell
void dijkstra() {
    printf("starting dijkstra\n");

    //initializes disances array to infinity
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            distances[y][x] = INF;
        }
    }

    //pick starting cell as center
    Cell* center = graph[MAZE_HEIGHT/2][MAZE_WIDTH/2];
    Cell* moving = center;

    //intial condition
    center->distance = 0;
    distances[MAZE_HEIGHT/2][MAZE_WIDTH/2] = 0;
    
    while (allNines()) { //loop until nowhere else to go
        Cell* nearest = NULL;
        moving->visited = true; //set current to be visited 
        distances[moving->y][moving->x] = INF; //remove from feasible distances to be explored in the future
        //printf("coordinates of star vertex: x: %d, y: %d\n", moving->x, moving->y);

        //check all four neighbours and not leaving though the edge
        if (((moving->cellVal & CELL_PATH_N) == CELL_PATH_N) && (moving->y != 0)) {
            if ((moving->upNeigh)->visited == false) { //ensures has not yet found a guaruanteed shortest path to start
                //cell to go to has already been seen
                if ((moving->upNeigh)->previous != NULL) {
                    //compare distance through new path with distance from other path and adjust if needed
                    if (((moving->distance) + 1) < ((moving->upNeigh)->previous)->distance) {
                        (moving->upNeigh)->distance = (moving->distance) + 1;
                        (moving->upNeigh)->previous = moving; //save previous 
                    }
                //cell to go to has not yet been seen
                } else if (((moving->upNeigh)->previous == NULL) && (moving->upNeigh != center)) { //second condtion ensures is not trying to go to center because center's previous will always be NULL
                    (moving->upNeigh)->distance = (moving->distance) + 1; //only one option for distance
                    (moving->upNeigh)->previous = moving; //save previous
                }
                //add the calcuated potential distance to the distances array
                distances[(moving->upNeigh)->y][(moving->upNeigh)->x] = (moving->upNeigh)->distance;
            }
        }
        //same comments as above
        if (((moving->cellVal & CELL_PATH_S) == CELL_PATH_S) && (moving->y != (MAZE_HEIGHT - 1))) {
            if ((moving->downNeigh)->visited == false) {
                if ((moving->downNeigh)->previous != NULL) {
                    if (((moving->distance) + 1) < ((moving->downNeigh)->previous)->distance) {
                        (moving->downNeigh)->distance = (moving->distance) + 1;
                        (moving->downNeigh)->previous = moving;
                    }
                } else if (((moving->downNeigh)->previous == NULL) && (moving->downNeigh != center)) {
                    (moving->downNeigh)->distance = (moving->distance) + 1;
                    (moving->downNeigh)->previous = moving;
                }
                distances[(moving->downNeigh)->y][(moving->downNeigh)->x] = (moving->downNeigh)->distance;
            }
        }
        if (((moving->cellVal & CELL_PATH_W) == CELL_PATH_W) && (moving->x != 0)) {
            if ((moving->leftNeigh)->visited == false) {
                if ((moving->leftNeigh)->previous != NULL) {
                    if (((moving->distance) + 1) < ((moving->leftNeigh)->previous)->distance) {
                        (moving->leftNeigh)->distance = (moving->distance) + 1;
                        (moving->leftNeigh)->previous = moving;
                    }
                } else if (((moving->leftNeigh)->previous == NULL) && (moving->leftNeigh != center)) {
                    (moving->leftNeigh)->distance = (moving->distance) + 1;
                    (moving->leftNeigh)->previous = moving;
                }
            distances[(moving->leftNeigh)->y][(moving->leftNeigh)->x] = (moving->leftNeigh)->distance;
            }
        }
        if (((moving->cellVal & CELL_PATH_E) == CELL_PATH_E) && (moving->x != (MAZE_WIDTH-1))) {
            if ((moving->rightNeigh)->visited == false) {
                if ((moving->rightNeigh)->previous != NULL) {
                    if (((moving->distance) + 1) < ((moving->rightNeigh)->previous)->distance) {
                        (moving->rightNeigh)->distance = (moving->distance) + 1;
                        (moving->rightNeigh)->previous = moving;
                    }
                } else if (((moving->rightNeigh)->previous == NULL) && (moving->rightNeigh != center)) {
                    (moving->rightNeigh)->distance = (moving->distance) + 1;
                    (moving->rightNeigh)->previous = moving;
                }
            distances[(moving->rightNeigh)->y][(moving->rightNeigh)->x] = (moving->rightNeigh)->distance;
            }
        } 

        int smallestD = INF; //reset smallest to INF

        //printDists(); //check print of distances array

        //loop through distances array looking for the shortest distance (next cell to visit)
        for (int y = 0; y < MAZE_HEIGHT; y++) {
            for (int x = 0; x < MAZE_WIDTH; x++) {
                if (distances[y][x] < smallestD) {
                    smallestD = distances[y][x];
                    nearest = graph[y][x];
                }
            }
        }

        moving = nearest; //move to the closest cell
        //printf("%d\n", smallestD);
    }
}

//function that follows back from exits to center with position pointers as calculated by Dijkstra's
int findPaths() {
    printf("in find paths\n");
    //cellNum indicates where along the path cells are being added, pathL keeps track of current shortest path to an exit
    int cellNum, pathL = INF;

    //evaluate Dijkstra's for evey exit
    for (int i = 0; i < exitsFound; i++) {

        printf("working with exit %d\n", i);
        printf("the exit is at x = %d, y = %d\n", exitLocations[i]->x, exitLocations[i]->y);

        Cell* current = exitLocations[i]; //start at the exit
        cellNum = 0;
        do{  //loop till reaches the middle
            paths[i][0][cellNum] = current->x;
            paths[i][1][cellNum] = current->y;
            cellNum++;
            current = current->previous; //move along path

        } while (current != graph[MAZE_HEIGHT/2][MAZE_WIDTH/2]);
        paths[i][0][cellNum] = MAZE_WIDTH/2; //make sure origin gets included
        paths[i][1][cellNum] = MAZE_HEIGHT/2;

        printf("Length: %d\n", cellNum);
        //compare with current shortest path to exit
        if ((cellNum + 1) < pathL) {
            shortestE = i;
            printf("Number of the new short path: %d\n", shortestE);
            pathL = cellNum + 1;
            printf("Length of that path: %d\n", pathL);
        }
    }
    return pathL; //pathL, actual number of cells (starts at 1)
}

//function to follow along the shortest path, addind to pound sign bit to the affected cores
void displayPaths(int length) {
    for (int i = 0; i < length; i++) {
        //pull x's and y's out of the 3D path array
        maze[(get_index(paths[shortestE][0][i], paths[shortestE][1][i]))] |= CELL_COLOUR;
    }
} 

//main function
int main() {
    //generation
    generate_maze();
    addEntranceToBox();
    addExits();

    //print maze unsolved
    print_maze(); 

    //solving
    generate_graph();
    printf("Num exits: %d\n", exitsFound);
    dijkstra();
    int pathL = findPaths();
    displayPaths(pathL);

    //print solved maze
    print_maze();
}