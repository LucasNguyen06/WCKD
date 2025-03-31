#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
//#include <gtk/gtk.h>

//Define the size of the maze, can be change
#define MAZE_WIDTH 11 // DO ODD
#define MAZE_HEIGHT 11 //DO ODD
#define INF 9999

//Paths: North, South, East, and West
//Using hexidecimal as bit flag
//Cause it memory efficient as it can store multiple direction in a single int using bitwise operation
enum{
    CELL_PATH_N = 0x01, //North(up) = 00 0001(binary)
    CELL_PATH_S = 0x02, //South(down) = 00 0010
    CELL_PATH_E = 0x04, //East(left) = 00 0100
    CELL_PATH_W = 0x08, //West(right) = 00 1000
    CELL_VISITED = 0x10, //Visisted cells = 01 0000
    CELL_EXIT = 0x20, //cell is one of the exits 100 0000
    CELL_COLOUR = 0x40, //cell is in an exit path 100 0000
};

//Coordinate structure to store x and y
typedef struct coords{
    int x, y;
}Point;

typedef struct cell{
    int x, y;
    int cellVal;
    struct cell* upNeigh; //neighbor 1
    struct cell* downNeigh; //neighbor 2
    struct cell* leftNeigh; //neightbor 3
    struct cell* rightNeigh; //neighbor 4
    int originator; //0 if start (center)
    int distance;
    bool visited;
    struct cell* previous;
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

//Check if stack is empty, then return 0
bool isEmpty(){
   return stack_size == 0;
}

//PUSH function, return updated count
void pushP(Point p){
    stack[stack_size++] = p;
}

//POP Function, return updated count
Point popP(){
    return stack[--stack_size];
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

            maze[index] |= CELL_VISITED;
            visited_cell++;
        }
    }

    maze[get_index(start_x + 1, start_y)] |= CELL_PATH_N;
    maze[get_index(start_x + 1, start_y)] |= CELL_PATH_S;
    maze[get_index(start_x + 1, start_y + 2)] |= CELL_PATH_N;
    maze[get_index(start_x + 1, start_y + 2)] |= CELL_PATH_S; //was N
    maze[get_index(start_x, start_y + 1)] |= CELL_PATH_E;
    maze[get_index(start_x, start_y + 1)] |= CELL_PATH_W;
    maze[get_index(start_x + 2, start_y + 1)] |= CELL_PATH_E;
    maze[get_index(start_x + 2, start_y + 1)] |= CELL_PATH_W;
    
    maze[get_index(start_x + 1, start_y + 1)] |= CELL_PATH_N;
    maze[get_index(start_x + 1, start_y + 1)] |= CELL_PATH_S;
    maze[get_index(start_x + 1, start_y + 1)] |= CELL_PATH_E;
    maze[get_index(start_x + 1, start_y + 1)] |= CELL_PATH_W;
    

    /*
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            //get coordinates of the cell in the box
            int x = start_x + j;
            int y = start_y + i;
            int index = get_index(x, y);

            maze[index] |= CELL_VISITED;
            visited_cell++;
            //removes wall inside box, creating an empty box
            
            if (i > 0) maze[index] |= CELL_PATH_N;
            if (i < 2) maze[index] |= CELL_PATH_S;
            if (j > 0) maze[index] |= CELL_PATH_W;
            if (j < 2) maze[index] |= CELL_PATH_E;
            
        }
    }
    */
}

//Maze generating algorithm: DFS, recursive backtracking
void generate_maze(){
    memset(maze, 0, sizeof(maze));
    srand(time(NULL));

    intialize_center_box();

    //starting position
    Point start = {MAZE_WIDTH/2, MAZE_HEIGHT/2}; //generate maze right in the middle
    pushP(start); //starting cell push onto stack
    //Starting cell is marked visited, update visited cell
    maze[get_index(start.x, start.y)] |= CELL_VISITED;
    visited_cell++;

    //While loop to visit all cells on the grid
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
            //Case 1:
            case 0:
                maze[get_index(x, y - 1)] |= CELL_VISITED | CELL_PATH_S;
                maze[get_index(x, y)] |= CELL_PATH_N;
                pushP((Point){x, y - 1});
                break;
            case 1://Case 2:
                maze[get_index(x + 1, y)] |= CELL_VISITED | CELL_PATH_W;
                maze[get_index(x, y)] |= CELL_PATH_E;
                pushP((Point){x + 1, y});
                break;
            case 2: //Case 3:
                maze[get_index(x, y + 1)] |= CELL_VISITED | CELL_PATH_N;
                maze[get_index(x, y)] |= CELL_PATH_S;
                pushP((Point){x, y + 1});
                break;
            case 3: //Case 4:
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

void unfill(int outputX, int outputY) {
    visual_maze[outputX][outputY] = ' ';
}

void print_maze() {
    int workingX = 0;
    int workingY = 0;
    for (int y = 0; y < (MAZE_HEIGHT*2)+1; y++) { //progess down columns (from one full row to the next)
            for (int x = 0; x < (MAZE_WIDTH*2)+1; x++) { //go across rows (from one column to the next)
                visual_maze[y][x] = '*';
            }
        }

    for (int y = 0; y < MAZE_HEIGHT; y++) { //by row
        for (int x = 0; x < MAZE_WIDTH; x++) { //by column

            printf("%3d ", maze[get_index(x,y)]); //for printing the maze gen number to check 

            workingX = 2*x + 1;
            workingY = 2*y + 1;

            int walls = maze[get_index(x, y)];

            if ((maze[get_index(x, y)] & CELL_COLOUR) == CELL_COLOUR) {
                visual_maze[workingX][workingY] = '#';
            } else {
                unfill(workingX, workingY);
            }

            if ((walls & CELL_PATH_N) == CELL_PATH_N)  //north
                unfill(workingX, workingY-1);
            
            if ((walls & CELL_PATH_S) == CELL_PATH_S)  //south
                unfill(workingX, workingY+1);
            
            if ((walls & CELL_PATH_E) == CELL_PATH_E)  //east
                unfill(workingX+1, workingY);
            
            if ((walls & CELL_PATH_W) == CELL_PATH_W)  //west
                unfill(workingX-1, workingY);
        }
        printf("\n"); //also for check printing
    } 

    printf("\033[1;32m");

    for (int y = 0; y < (MAZE_HEIGHT*2)+1; y++) { //how many rows to print
        for (int x = 0; x < (MAZE_WIDTH*2)+1; x++) { //along row
            if ((visual_maze[x][y] == '#') || ((visual_maze[x][y] == ' ') && (visual_maze[x-1][y] == '#') && (visual_maze[x+1][y] == '#')) || ((visual_maze[x][y] == ' ') && (visual_maze[x][y-1] == '#') && (visual_maze[x][y+1] == '#'))) { //j i flipped down here
                printf("\033[1;31m");
                printf("# ");
                printf("\033[1;32m");
           } else if ((y > ((MAZE_WIDTH*2)+1)/2 - 4) && (y < ((MAZE_WIDTH*2)+1)/2 + 4) && (x > ((MAZE_HEIGHT*2)+1)/2 - 4) && (x < ((MAZE_HEIGHT*2)+1)/2 + 4)) {
                printf("\033[1;37m");
                printf("%c ", visual_maze[x][y]);
                printf("\033[1;32m");
            } else {
                printf("%c ", visual_maze[x][y]);
            }
        }
    printf("\n");
    }
    printf("\033[0m ");
    
}

//Add entrance function to the 3x3 box. Will add entrance on each side of the box, in the middle
void addEntranceToBox(){
    //Top left corner of 3x3
    int start_x = MAZE_WIDTH/2 - 1;
    int start_y = MAZE_HEIGHT/2 - 1;

    //Top side entrance
    maze[get_index(start_x + 1, start_y - 1)] |= CELL_VISITED | CELL_PATH_S;
    unfill((start_y * 2) - 1, (start_x * 2) + 3);

    // Bottom side entrance
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
    int topExit = (rand() % (MAZE_WIDTH - 2)) + 1;
    maze[get_index(topExit, 0)] |= CELL_PATH_N;
    maze[get_index(topExit, 0)] |= CELL_EXIT;
    unfill(0, (topExit * 2) + 1);//paths occupies odd number, so +1 after *2. Repeat for all

    //Bottom side exit, y = -1
    int bottomExit = (rand() % (MAZE_WIDTH - 2)) + 1;
    maze[get_index(bottomExit, MAZE_HEIGHT - 1)] |= CELL_PATH_S;
    maze[get_index(bottomExit, MAZE_HEIGHT -1 )] |= CELL_EXIT;
    unfill(MAZE_HEIGHT * 2, (bottomExit * 2) +1);

    //Left side exit, x = 0
    int leftExit = (rand() % (MAZE_HEIGHT - 2)) + 1;
    maze[get_index(0, leftExit)] |= CELL_PATH_W;
    maze[get_index(0, leftExit)] |= CELL_EXIT;
    unfill((leftExit * 2) + 1, 0);

    //right side exit, x = -1
    int rightExit = (rand() % (MAZE_HEIGHT - 2)) + 1;
    maze[get_index(MAZE_WIDTH - 1, rightExit)] |= CELL_PATH_E;
    maze[get_index(MAZE_WIDTH - 1, rightExit)] |= CELL_EXIT;
    unfill((rightExit * 2) + 1, MAZE_WIDTH * 2);
}

Cell* createCell(int x, int y) {
    //printf("MADE IT IN\n");
    //printf("MADE IT PAST\n");
    Cell* newCell = (Cell*)malloc(sizeof(Cell));
    newCell->x = x;
    newCell->y = y;
    newCell->cellVal = maze[get_index(x, y)];
    newCell->upNeigh = NULL;
    newCell->downNeigh = NULL;
    newCell->leftNeigh = NULL;
    newCell->rightNeigh = NULL;
    newCell->originator = 0;
    newCell->distance = INF;
    newCell->visited = false;
    newCell->previous = NULL;
    return newCell;
}

//function will move through the entire maze, allowing for cycles, and keep track of exit coordinates (but won't stop when it finds them)
void generate_graph() {
    printf("started graphing\n");

    for (int y = 0; y < MAZE_HEIGHT; y++) { //down rows
        for (int x = 0; x < MAZE_WIDTH; x++) { //across columns
            Cell* current = createCell(x, y);
            graph[y][x] = current;
        }
    }

    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            Cell* current = graph[y][x];
            if ((current->cellVal & CELL_EXIT) == CELL_EXIT) {
                //if exit found
                exitLocations[exitsFound] = current;
                exitsFound++;
                printf("An exit found at y=%d, x=%d!\n", y, x);
            }
            if (((current->cellVal & CELL_PATH_N) == CELL_PATH_N) && (y != 0)) {
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

void printDists() {
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            printf("%4d ", distances[y][x]);
        }
        printf("\n");
    }
}

bool allNines() {
    bool foundIssue = false;
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            if (distances[y][x] != 9999) {
                foundIssue = true;
            }
        }
    }
    return foundIssue;
}
//function will run dijkstra's on the graph given the graph and the destination 
void dijkstra() {
    printf("starting dijkstra\n");
    Cell* center = graph[MAZE_HEIGHT/2][MAZE_WIDTH/2];
    Cell* moving = center;
    center->distance = 0;
    distances[MAZE_HEIGHT/2][MAZE_WIDTH/2] = 0;
    //variables for where to go next
    
    while (allNines()) { //number of passes
        Cell* nearest = NULL;
        moving->visited = true;
        distances[moving->y][moving->x] = INF;
        printf("coordinates of star vertex: x: %d, y: %d\n", moving->x, moving->y);
        //check all four neighbours
        if (((moving->cellVal & CELL_PATH_N) == CELL_PATH_N) && (moving->y != 0)) {
            printf("trying north\n");
            if ((moving->upNeigh)->visited == false) {
                if ((moving->upNeigh)->previous != NULL) {
                    printf("this already has been seen\n");
                    if (((moving->distance) + 1) < ((moving->upNeigh)->previous)->distance) {
                        printf("better distance\n");
                        (moving->upNeigh)->distance = (moving->distance) + 1;
                        (moving->upNeigh)->previous = moving;
                    }
                } else if (((moving->upNeigh)->previous == NULL) && (moving->upNeigh != center)) {
                    printf("new (not seen)\n");
                    (moving->upNeigh)->distance = (moving->distance) + 1;
                    (moving->upNeigh)->previous = moving;
                }
                distances[(moving->upNeigh)->y][(moving->upNeigh)->x] = (moving->upNeigh)->distance;
            }
        }
        if (((moving->cellVal & CELL_PATH_S) == CELL_PATH_S) && (moving->y != (MAZE_HEIGHT - 1))) {
            printf("trying south\n");
            if ((moving->downNeigh)->visited == false) {
                if ((moving->downNeigh)->previous != NULL) {
                    printf("this already has been seen\n");
                    if (((moving->distance) + 1) < ((moving->downNeigh)->previous)->distance) {
                        printf("better distance\n");
                        (moving->downNeigh)->distance = (moving->distance) + 1;
                        (moving->downNeigh)->previous = moving;
                    }
                } else if (((moving->downNeigh)->previous == NULL) && (moving->downNeigh != center)) {
                    printf("new (not seen)\n");
                    (moving->downNeigh)->distance = (moving->distance) + 1;
                    (moving->downNeigh)->previous = moving;
                }
                distances[(moving->downNeigh)->y][(moving->downNeigh)->x] = (moving->downNeigh)->distance;
            }
        }
        if (((moving->cellVal & CELL_PATH_W) == CELL_PATH_W) && (moving->x != 0)) {
            printf("trying west\n");
            if ((moving->leftNeigh)->visited == false) {
                if ((moving->leftNeigh)->previous != NULL) {
                    printf("this already has been seen\n");
                    if (((moving->distance) + 1) < ((moving->leftNeigh)->previous)->distance) {
                        printf("better distance\n");
                        (moving->leftNeigh)->distance = (moving->distance) + 1;
                        (moving->leftNeigh)->previous = moving;
                    }
                } else if (((moving->leftNeigh)->previous == NULL) && (moving->leftNeigh != center)) {
                    printf("new (not seen)\n");
                    (moving->leftNeigh)->distance = (moving->distance) + 1;
                    (moving->leftNeigh)->previous = moving;
                }
            distances[(moving->leftNeigh)->y][(moving->leftNeigh)->x] = (moving->leftNeigh)->distance;
            }
        }
        if (((moving->cellVal & CELL_PATH_E) == CELL_PATH_E) && (moving->x != (MAZE_WIDTH-1))) {
            printf("trying east\n");
            if ((moving->rightNeigh)->visited == false) {
                if ((moving->rightNeigh)->previous != NULL) {
                    printf("this already has been seen\n");
                    if (((moving->distance) + 1) < ((moving->rightNeigh)->previous)->distance) {
                        printf("better distance\n");
                        (moving->rightNeigh)->distance = (moving->distance) + 1;
                        (moving->rightNeigh)->previous = moving;
                    }
                } else if (((moving->rightNeigh)->previous == NULL) && (moving->rightNeigh != center)) {
                    printf("new (not seen)\n");
                    (moving->rightNeigh)->distance = (moving->distance) + 1;
                    (moving->rightNeigh)->previous = moving;
                }
            distances[(moving->rightNeigh)->y][(moving->rightNeigh)->x] = (moving->rightNeigh)->distance;
            }
        } 

        int smallestD = INF;

        printDists();

        for (int y = 0; y < MAZE_HEIGHT; y++) {
            for (int x = 0; x < MAZE_WIDTH; x++) {
                if (distances[y][x] < smallestD) {
                    smallestD = distances[y][x];
                    nearest = graph[y][x];
                }
            }
        }

        moving = nearest;
        printf("%d\n", smallestD);
    }
}

//function will run dijskstra's four times and determine which path is the shortest. This path will be passed into the display path functoin
int findPaths() {
    printf("in find paths\n");
    int cellNum, pathL = INF;

    for (int i = 0; i < exitsFound; i++) {
        printf("working with exit %d\n", i);
        printf("the exit is at x = %d, y = %d\n", exitLocations[i]->x, exitLocations[i]->y);
        Cell* current = exitLocations[i]; //start at the exit
        cellNum = 0;
        do{  //loop till reaches the middle
            printf("in do while\n");
            printf("current x: %d, current y: %d\n", current->x, current->y);
            paths[i][0][cellNum] = current->x;
            paths[i][1][cellNum] = current->y;
            cellNum++;
            printf("previous x: %d, previous y: %d\n", (current->previous)->x, (current->previous)->y);
            current = current->previous;
        } while (current != graph[MAZE_HEIGHT/2][MAZE_WIDTH/2]);
        paths[i][0][cellNum] = MAZE_WIDTH/2; //make sure origin gets included
        paths[i][1][cellNum] = MAZE_HEIGHT/2;

        printf("Length: %d\n", cellNum);
        if ((cellNum + 1) < pathL) {
            shortestE = i;
            printf("Number of the new short path: %d\n", shortestE);
            pathL = cellNum + 1;
            printf("Length of that path: %d\n", pathL);
        }
    }
    return pathL; //pathL, actual number of cells (starts at 1)
}

void displayPaths(int length) {
    for (int i = 0; i < length; i++) {
        maze[(get_index(paths[shortestE][0][i], paths[shortestE][1][i]))] |= CELL_COLOUR;
    }
} 

 
int main() {
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            distances[y][x] = 9999;
        }
    }
    generate_maze();
    printf("in main single check\n");
    addEntranceToBox();
    addExits();
    print_maze();
    printf("finished printing\n");
    generate_graph();
    printf("Num exits: %d\n", exitsFound);
    dijkstra();
    printf("dijkstra done\n");
    int pathL = findPaths();
    displayPaths(pathL);
    print_maze();
}