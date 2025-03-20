#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
//#include <gtk/gtk.h>

//Define the size of the maze, can be change
#define MAZE_WIDTH 30
#define MAZE_HEIGHT 30
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
    CELL_EXIT = 0x32, //cell is one of the exits 10 0000
};

//Coordinate structure to store x and y
typedef struct coords{
    int x, y;
}Point;

//array to store cells current state(i.e visited?)
int maze[MAZE_WIDTH * MAZE_HEIGHT];
//stack for back tracking
Point stack[MAZE_WIDTH * MAZE_HEIGHT];
//track number of elements in stack
int stack_size = 0;
//track number of visited cells
int visited_cell = 0;
//declare visual maze of be displayed
char visual_maze[(MAZE_HEIGHT*2)+1][(MAZE_WIDTH*2)+1];

//Check if stack is empty, then return 0
bool isEmpty(){
   return stack_size == 0;
}

//PUSH function, return updated count
void push(Point p){
    stack[stack_size++] = p;
}

//POP Function, return updated count
Point pop(){
    return stack[--stack_size];
}

//Function that converts 2D array to 1D array index.
//For easier access to 2D grid using 1D array
int get_index(int x, int y){
    return y * MAZE_WIDTH + x;
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
            //removes wall inside box, creating an empty box
            if (i > 0) maze[index] |= CELL_PATH_N;
            if (i < 2) maze[index] |= CELL_PATH_S;
            if (j > 0) maze[index] |= CELL_PATH_W;
            if (j < 2) maze[index] |= CELL_PATH_E;
        }
    }
}

//Maze generating algorithm: DFS, recursive backtracking
void generate_maze(){
    memset(maze, 0, sizeof(maze));
    srand(time(NULL));

    intialize_center_box();

    //starting position
    Point start = {MAZE_WIDTH/2, MAZE_HEIGHT/2}; //generate maze right in the middle
    push(start); //starting cell push onto stack
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
                push((Point){x, y - 1});
                break;
            case 1://Case 2:
                maze[get_index(x + 1, y)] |= CELL_VISITED | CELL_PATH_W;
                maze[get_index(x, y)] |= CELL_PATH_E;
                push((Point){x + 1, y});
                break;
            case 2: //Case 3:
                maze[get_index(x, y + 1)] |= CELL_VISITED | CELL_PATH_N;
                maze[get_index(x, y)] |= CELL_PATH_S;
                push((Point){x, y + 1});
                break;
            case 3: //Case 4:
                maze[get_index(x - 1, y)] |= CELL_VISITED | CELL_PATH_E;
                maze[get_index(x, y)] |= CELL_PATH_W;
                push((Point){x - 1, y});
                break;
            }
            visited_cell++;
        } else {
            pop();
        }  
    }
}

void unfill(int outputX, int outputY) {
    visual_maze[outputX][outputY] = ' ';
}
void print_maze() {
    int workingX = 0;
    int workingY = 0;
    for (int i = 0; i < (MAZE_HEIGHT*2)+1; i++) { //go down columns
            for (int j = 0; j < (MAZE_WIDTH*2)+1; j++) { //go across rows
                visual_maze[i][j] = '*';
            }
        }

        //Clear internal walls inside the 3x3 box
        int startX = (MAZE_WIDTH / 2) * 2 - 1;
        int startY = (MAZE_HEIGHT / 2) * 2 - 1;
        //iterate 5 times as we want to clear only the internal walls of the box, leaving the border of 3x3 box walls untouch
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < 5; j++) {
                unfill(startY + i, startX + j);
            }
        }


    for (int i = 0; i < MAZE_HEIGHT; i++) { //by column
        for (int j = 0; j < MAZE_WIDTH; j++) { //by row
            workingX = 2*j + 1;
            workingY = 2*i + 1;

            int walls = maze[get_index(j, i)];
            unfill(workingX, workingY);

            if ((walls & CELL_PATH_N) == CELL_PATH_N)  //north
                unfill(workingX, workingY-1);
            
            if ((walls & CELL_PATH_S) == CELL_PATH_S)  //south
                unfill(workingX, workingY+1);
            
            if ((walls & CELL_PATH_E) == CELL_PATH_E)  //east
                unfill(workingX+1, workingY);
            
            if ((walls & CELL_PATH_W) == CELL_PATH_W)  //west
                unfill(workingX-1, workingY);
        }
    } 

    printf("\033[1;32m");
    for (int i = 0; i < (MAZE_HEIGHT*2)+1; i++) {
        for (int j = 0; j < (MAZE_WIDTH*2)+1; j++) {
            printf("%c ", visual_maze[i][j]);
        }
    printf("\n");
    }   
    printf("\033[0m ");
}

//Add entrance function to the 3x3 box. Will add entrance on each side of the box, in the middle
void addEntranceToBox(){
    //Top left corner of 3x3
    int start_x = MAZE_WIDTH/2 - 1;
    int start_y = MAZE_HEIGHT/2 -1;

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
    int topExit = rand() % MAZE_WIDTH;
    maze[get_index(topExit, 0)] |= CELL_PATH_N;
    maze[get_index(topExit, 0)] |= CELL_EXIT;
    unfill(0, (topExit * 2) + 1);//paths occupies odd number, so +1 after *2. Repeat for all

    //Bottom side exit, y = -1
    int bottomExit = rand() % MAZE_WIDTH;
    maze[get_index(bottomExit, MAZE_HEIGHT - 1)] |= CELL_PATH_S;
    maze[get_index(bottomExit, MAZE_HEIGHT -1 )] |= CELL_EXIT;
    unfill(MAZE_HEIGHT * 2, (bottomExit * 2) +1);

    //Left side exit, x = 0
    int leftExit = rand() % MAZE_HEIGHT;
    maze[get_index(0, leftExit)] |= CELL_PATH_W;
    maze[get_index(0, leftExit)] |= CELL_EXIT;
    unfill((leftExit * 2) + 1, 0);

    //right side exit, x = -1
    int rightExit = rand() % MAZE_HEIGHT;
    maze[get_index(MAZE_WIDTH - 1, rightExit)] |= CELL_PATH_E;
    maze[get_index(MAZE_WIDTH - 1, rightExit)] |= CELL_EXIT;
    unfill((rightExit * 2) + 1, MAZE_WIDTH * 2);
}

int main() {
    generate_maze();
    printf("in main double check\n");
    addEntranceToBox();
    addExits();
    print_maze();
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
