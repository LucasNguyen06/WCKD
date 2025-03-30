#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
//#include <gtk/gtk.h>

//Define the size of the maze, can be change
int MAZE_WIDTH;// DO ODD
int MAZE_HEIGHT; //DO ODD
#define CELL_SIZE 20 //Cell size 20x20 pixels in GTK window

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
}Cell;

//array to store cells current state(i.e visited?)

//2D array of exit locations
Cell* exitLocations[4];
//stack for gen back tracking

//track number of elements in stackC
int stackC_size = 0;
//track number of elements in stack
int stack_size = 0;
//track number of visited cells
int visited_cell = 0;
//keep track of closest exit
int shortestE = 0;
//number of locatable exits
int exitsFound = 0;
//declare visual maze of be displayed


int exits [2][4];

//Check if stack is empty, then return 0
bool isEmpty(){
   return stack_size == 0;
}

//PUSH function, return updated count
void pushP(Point p, Point stack[]){
    stack[stack_size++] = p;
}

//POP Function, return updated count
Point popP(Point stack[]){
    return stack[--stack_size];
}

void pushC(Cell *c, Cell *stackC[]) {
    stackC[stackC_size++] = c;
}

Cell* popC(Cell* stackC[]) {
    return stackC[--stackC_size];
}

//Function that converts 2D array to 1D array index.
//For easier access to 2D grid using 1D array
int get_index(int x, int y){
    return y * MAZE_WIDTH + x;
}

//Intialize starting center box, 3x3
void intialize_center_box(int maze[]){
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
    maze[get_index(start_x + 1, start_y + 2)] |= CELL_PATH_N;
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
void generate_maze(int maze[],Point stack[]){
    memset(maze, 0, sizeof(maze));
    srand(time(NULL));

    intialize_center_box(maze);

    //starting position
    Point start = {MAZE_WIDTH/2, MAZE_HEIGHT/2}; //generate maze right in the middle
    pushP(start,stack); //starting cell push onto stack
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
                pushP((Point){x, y - 1},stack);
                break;
            case 1://Case 2:
                maze[get_index(x + 1, y)] |= CELL_VISITED | CELL_PATH_W;
                maze[get_index(x, y)] |= CELL_PATH_E;
                pushP((Point){x + 1, y},stack);
                break;
            case 2: //Case 3:
                maze[get_index(x, y + 1)] |= CELL_VISITED | CELL_PATH_N;
                maze[get_index(x, y)] |= CELL_PATH_S;
                pushP((Point){x, y + 1},stack);
                break;
            case 3: //Case 4:
                maze[get_index(x - 1, y)] |= CELL_VISITED | CELL_PATH_E;
                maze[get_index(x, y)] |= CELL_PATH_W;
                pushP((Point){x - 1, y},stack);
                break;
            }
            visited_cell++;
        } else {
            popP(stack);
        }  
    }
}

void unfill(int outputX, int outputY, char *visual_maze[]) {
    visual_maze[outputX][outputY] = ' ';
}

void print_maze(int maze[],char *visual_maze[]) {
    int workingX = 0;
    int workingY = 0;
    for (int i = 0; i < (MAZE_HEIGHT*2)+1; i++) { //go down columns
            for (int j = 0; j < (MAZE_WIDTH*2)+1; j++) { //go across rows
                visual_maze[i][j] = '*';
            }
        }

        /*
        //Clear internal walls inside the 3x3 box
        int startX = (MAZE_WIDTH / 2) * 2 - 1;
        int startY = (MAZE_HEIGHT / 2) * 2 - 1;
        //iterate 5 times as we want to clear only the internal walls of the box, leaving the border of 3x3 box walls untouch
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                unfill(startY + i, startX + j);
            }
        }
        */



    for (int i = 0; i < MAZE_HEIGHT; i++) { //by row
        for (int j = 0; j < MAZE_WIDTH; j++) { //by column

            printf("%3d ", maze[get_index(j,i)]); //for printing the maze gen number to check 

            workingX = 2*j + 1;
            workingY = 2*i + 1;

            int walls = maze[get_index(j, i)];

            if ((maze[get_index(j, i)] & CELL_COLOUR) == CELL_COLOUR) {
                visual_maze[workingX][workingY] = '#';
            } else {
                unfill(workingX, workingY, visual_maze);
            }

            if ((walls & CELL_PATH_N) == CELL_PATH_N)  //north
                unfill(workingX, workingY-1,visual_maze);
            
            if ((walls & CELL_PATH_S) == CELL_PATH_S)  //south
                unfill(workingX, workingY+1,visual_maze);
            
            if ((walls & CELL_PATH_E) == CELL_PATH_E)  //east
                unfill(workingX+1, workingY,visual_maze);
            
            if ((walls & CELL_PATH_W) == CELL_PATH_W)  //west
                unfill(workingX-1, workingY,visual_maze);
        }
        printf("\n"); //also for check printing
    } 

    printf("\033[1;32m");

    for (int i = 0; i < (MAZE_HEIGHT*2)+1; i++) {
        for (int j = 0; j < (MAZE_WIDTH*2)+1; j++) {
            if ((visual_maze[j][i] == '#') || ((visual_maze[j][i] == ' ') && (visual_maze[j-1][i] == '#') && (visual_maze[j+1][i] == '#')) || ((visual_maze[j][i] == ' ') && (visual_maze[j][i-1] == '#') && (visual_maze[j][i+1] == '#'))) { //j i flipped down here
                printf("\033[1;31m");
                printf("# ");
                printf("\033[1;32m");
           } else if ((i > ((MAZE_WIDTH*2)+1)/2 - 4) && (i < ((MAZE_WIDTH*2)+1)/2 + 4) && (j > ((MAZE_HEIGHT*2)+1)/2 - 4) && (j < ((MAZE_HEIGHT*2)+1)/2 + 4)) {
                printf("\033[1;37m");
                printf("%c ", visual_maze[j][i]);
                printf("\033[1;32m");
            } else {
                printf("%c ", visual_maze[j][i]);
            }
        }
    printf("\n");
    }   
    printf("\033[0m ");
}

//Add entrance function to the 3x3 box. Will add entrance on each side of the box, in the middle
void addEntranceToBox(int maze[], char *visual_maze[]){
    //Top left corner of 3x3
    int start_x = MAZE_WIDTH/2 - 1;
    int start_y = MAZE_HEIGHT/2 - 1;

    //Top side entrance
    maze[get_index(start_x + 1, start_y - 1)] |= CELL_VISITED | CELL_PATH_S;
    unfill((start_y * 2) - 1, (start_x * 2) + 3,visual_maze);

    // Bottom side entrance
    maze[get_index(start_x + 1, start_y + 3)] |= CELL_VISITED | CELL_PATH_N;
    unfill((start_y * 2) + 5, (start_x * 2) + 3,visual_maze);

    // Left side entrance
    maze[get_index(start_x - 1, start_y + 1)] |= CELL_VISITED | CELL_PATH_E;
    unfill((start_y * 2) + 3, (start_x * 2) - 1,visual_maze);

    // Right side entrance
    maze[get_index(start_x + 3, start_y + 1)] |= CELL_VISITED | CELL_PATH_W;
    unfill((start_y * 2) + 3, (start_x * 2) + 7,visual_maze);
}

//Function to add entrance, 4 entrance, 1 on each side of the maze, random position
void addExits(int maze[], char *visual_maze[]){
    srand(time(NULL)); //ensures different result every run

    //Top side exit, y= 0
    int topExit = (rand() % (MAZE_WIDTH - 2)) + 1;
    maze[get_index(topExit, 0)] |= CELL_PATH_N;
    maze[get_index(topExit, 0)] |= CELL_EXIT;
    unfill(0, (topExit * 2) + 1,visual_maze);//paths occupies odd number, so +1 after *2. Repeat for all

    //Bottom side exit, y = -1
    int bottomExit = (rand() % (MAZE_WIDTH - 2)) + 1;
    maze[get_index(bottomExit, MAZE_HEIGHT - 1)] |= CELL_PATH_S;
    maze[get_index(bottomExit, MAZE_HEIGHT -1 )] |= CELL_EXIT;
    unfill(MAZE_HEIGHT * 2, (bottomExit * 2) +1,visual_maze);

    //Left side exit, x = 0
    int leftExit = (rand() % (MAZE_HEIGHT - 2)) + 1;
    maze[get_index(0, leftExit)] |= CELL_PATH_W;
    maze[get_index(0, leftExit)] |= CELL_EXIT;
    unfill((leftExit * 2) + 1, 0,visual_maze);

    //right side exit, x = -1
    int rightExit = (rand() % (MAZE_HEIGHT - 2)) + 1;
    maze[get_index(MAZE_WIDTH - 1, rightExit)] |= CELL_PATH_E;
    maze[get_index(MAZE_WIDTH - 1, rightExit)] |= CELL_EXIT;
    unfill((rightExit * 2) + 1, MAZE_WIDTH * 2,visual_maze);
}

Cell* createCell(int maze[],int x, int y, int *added[]) {
    //printf("MADE IT IN\n");
    if (added[x][y] == 1) return NULL;
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
    added[x][y] = 1;
    return newCell;
}

void solve_maze(int maze[], Cell* stackC, int *added[]) {
    Cell* center = createCell(maze,(MAZE_WIDTH/2), (MAZE_HEIGHT/2), added);
    Cell* explorer = center;
    Cell* temp; //for parent storing thing
  
    pushC(center, stackC);

    while ((exitsFound < 4) && (stackC_size > 0)) { //
        //printf("looping\n");
        if ((explorer->cellVal & CELL_EXIT) == CELL_EXIT) {
            //if exit found
            exitLocations[exitsFound] = explorer;
            exits[0][exitsFound]= explorer->x;
            exits[1][exitsFound]= explorer->y;
            exitsFound++;
            printf("An exit found at x=%d, y=%d!\n", explorer->x, explorer->y);
            maze[get_index(explorer->x,explorer->y)] -= 32;
           
            explorer = center;
            memset(added,0,sizeof(added));


        } else {
            if (((explorer->cellVal & CELL_PATH_N) == CELL_PATH_N) && (added[explorer->x][explorer->y-1] == 0)) {
                //printf("moving north\n");
                //printf("at %d going to %d\n\n", maze[get_index(explorer->x, explorer->y)], maze[get_index(explorer->x, explorer->y-1)]);
                explorer->upNeigh = createCell(maze, explorer->x, explorer->y-1, added);
                if (explorer->upNeigh == NULL) continue; //in case of the cycle error (redundant)
                (explorer->upNeigh)->originator = 2; //came from south
                temp = explorer;
                explorer = explorer->upNeigh;
                explorer->downNeigh = temp;
                pushC(explorer, stackC);
            } else if (((explorer->cellVal & CELL_PATH_S) == CELL_PATH_S) && (added[explorer->x][explorer->y+1] == 0)) {
                //printf("moving south\n");
                //printf("at %d going to %d\n\n", maze[get_index(explorer->x, explorer->y)], maze[get_index(explorer->x, explorer->y+1)]);
                explorer->downNeigh = createCell(maze,explorer->x, explorer->y+1, added);
                if (explorer->downNeigh == NULL) continue; //in case of the cycle error
                (explorer->downNeigh)->originator = 1; //came from north
                temp = explorer;
                explorer = explorer->downNeigh;
                explorer->upNeigh = temp;
                pushC(explorer,stackC);
            } else if (((explorer->cellVal & CELL_PATH_W) == CELL_PATH_W) && (added[explorer->x-1][explorer->y] == 0)) {
                //printf("moving west\n");
                //printf("at %d going to %d\n\n", maze[get_index(explorer->x, explorer->y)], maze[get_index(explorer->x-1, explorer->y)]);
                explorer->leftNeigh = createCell(maze,explorer->x-1, explorer->y,added);
                if (explorer->leftNeigh == NULL) continue; //in case of the cycle error
                (explorer->leftNeigh)->originator = 4; //came from east
                temp = explorer;
                explorer = explorer->leftNeigh;
                explorer->rightNeigh = temp;
                pushC(explorer,stackC);
            } else if (((explorer->cellVal & CELL_PATH_E) == CELL_PATH_E) && (added[explorer->x+1][explorer->y] == 0)) {
                //printf("moving east\n");
                //printf("at %d going to %d\n\n", maze[get_index(explorer->x, explorer->y)], maze[get_index(explorer->x+1, explorer->y)]);
                explorer->rightNeigh = createCell(maze,explorer->x+1, explorer->y,added);
                if (explorer->rightNeigh == NULL) continue; //in case of the cycle error
                (explorer->rightNeigh)->originator = 3; //came from west
                temp = explorer;
                explorer = explorer->rightNeigh;
                explorer->leftNeigh = temp;
                pushC(explorer,stackC);
            } else {
                //printf("Reached dead end or Full maze explored!\n");
                explorer = popC(stackC);
            }
        }  
    }
    printf("Maze solved!\n");
}

int findPaths(int **paths[]) {
    int cellNum, pathL = 99999;

    for (int i = 0; i < exitsFound; i++) {
        Cell* current = exitLocations[i]; //start at the exit
        cellNum = 0;
        do{  //loop till reaches the middle
            paths[i][0][cellNum] = current->x;
            paths[i][1][cellNum] = current->y;
            switch (current->originator) {
                case 1:              
                    current = current->upNeigh;
                    break;
                case 2:
                    current = current->downNeigh;
                    break;
                case 3: 
                    current = current->leftNeigh;
                    break;
                case 4:
                    current = current->rightNeigh;
                    break;
                default:
                    printf("colour error or end\n"); 
                    break;
            }
            cellNum++;
        } while (current->originator != 0);
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

void displayPaths(int maze[], int length, int **paths[]) {
    for (int i = 0; i < length; i++) {
        maze[(get_index(paths[shortestE][0][i], paths[shortestE][1][i]))] |= CELL_COLOUR;
    }
} 

 
int main() {
    printf("Enter Maze dimensions:");
    scanf("%d %d", &MAZE_WIDTH, &MAZE_HEIGHT);
    int maze[MAZE_WIDTH * MAZE_HEIGHT];
    //2D array for keeping track of if the cell has been added to tree
    int added[MAZE_HEIGHT][MAZE_WIDTH] = {0};
    //3D array to contain all 4 paths to exits
    int paths[4][2][MAZE_HEIGHT * MAZE_WIDTH] = {0};

    Point stack[MAZE_WIDTH * MAZE_HEIGHT];
    //stack for solving
    Cell* stackC[MAZE_WIDTH * MAZE_HEIGHT];
    char visual_maze[(MAZE_HEIGHT*2)+1][(MAZE_WIDTH*2)+1];

    generate_maze(maze, stack);
    printf("in main single check\n");
    addEntranceToBox(maze, visual_maze);
    addExits(maze, visual_maze);
    print_maze(maze,visual_maze);
    solve_maze(maze, stackC, added);
    printf("Num exits: %d\n", exitsFound);
    int pathL = findPaths(paths);
    displayPaths(maze,pathL,paths);
    print_maze(maze, visual_maze);
}
/*
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    generate_maze();
   
    GtkWidget *window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(window), "Maze Generator");
    gtk_window_set_default_size(GTK_WINDOW(window), MAZE_WIDTH * CELL_SIZE, MAZE_HEIGHT * CELL_SIZE);
   
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_maze, NULL, NULL);
   
    gtk_window_set_child(GTK_WINDOW(window), drawing_area);
    gtk_widget_show(window);
   
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_main();
   
    return 0;
}
    */
