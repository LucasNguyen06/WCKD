#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <gtk/gtk.h>

//Define the size of the maze, can be change
#define MAZE_WIDTH 50
#define MAZE_HEIGHT 50
#define CELL_SIZE 20 //Cell size 20x20 pixels in GTK window

//Paths: North, South, East, and West
//Using hexidecimal as bit flag
//Cause it memory efficient as it can store multiple direction in a single int using bitwise operation
enum{
    CELL_PATH_N = 0x01, //North(up) = 00001(binary)
    CELL_PATH_S = 0x02, //South(down) = 00010
    CELL_PATH_E = 0x04, //East(left) = 00100
    CELL_PATH_W = 0x08, //West(right) = 01000
    CELL_VISITED = 0x10, //Visisted cells = 10000
};

//Coordinate structure to store x and y
typedef struct coords Point;
typedef struct coords{
    int x, y;
}; 

//array to store cells current state(i.e visited?)
int maze[MAZE_WIDTH * MAZE_HEIGHT];
//stack for back tracking
Point stack[MAZE_WIDTH * MAZE_HEIGHT];
//track number of elements in stack
int stack_size = 0;
//track number of visited cells
int visited_cell = 0;

//Check if stack is empty, then return 0
bool isEmpty(){
   return stack_size == 0; 
}

//PUSH function, return updated count
void push(Point p){
    return stack[stack_size++] = p;
}

//POP Function, return updated count
void pop(){
    return stack[--stack_size];
}

//Function that converts 2D array to 1D array index. 
//For easier access to 2D grid using 1D array
int get_index(int x, int y){
    return y * MAZE_WIDTH + x;
}

//Maze generating algorithm: DFS, recursive backtracking
void generate_maze(){
    memset(maze, 0, sizeof(maze));
    srand(time(NULL));

    //starting position 
    Point start = { rand() % MAZE_WIDTH, rand() % MAZE_HEIGHT }; //randomly pick an x and y coordinate in the grid for starting position
    push(start); //starting cell push onto stack
    //Starting cell is marked visited, update visited cell
    maze[get_index(start.x, start.y)] |= CELL_VISITED;
    visited_cell = 1;

    //While loop to visit all cells on the grid
    while(visited_cell < MAZE_WIDTH * MAZE_HEIGHT) {
        Point top = stack[stack_size -1]; //top = start, where the algorithm going to start running
        int x = top.x;
        int y = top.y;
    
        int neighbors[4] = {0,1,2,3}; //neighbors cells around working cell. possible movement directions
        int count = 0;
        
        //Check if each direction has unvisted cell, and keep counts on how many valid neighbors to visit
        //Check if North is unvisited
        if(y > 0 && !(maze[get_index(x, y -1)] & CELL_VISITED))
            neighbors[count++]=0;
        //Check if East is unvisited
        if (x < MAZE_WIDTH - 1 && !(maze[(x + 1, y)] & CELL_VISITED))
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
            //Case 1: Moving north
            case 0:
                maze[get_index(x, y - 1)] |= CELL_VISITED | CELL_PATH_S; //remove the south wall of the new cell
                maze[get_index(x, y)] |= CELL_PATH_N; //remove the north wall of current cell
                push((Point){x, y - 1}); //push onto stack, move onto new cell
                break;
                //
            case 1: //Case 2: Moving east
                maze[get_index(x + 1, y)] |= CELL_VISITED | CELL_PATH_W; //remove west wall of the new cell
                maze[get_index(x, y)] |= CELL_PATH_E; //remove the east wall of the current cell
                push((Point){x + 1, y}); //push onto stack, move onto new cell
                break;
            case 2: //Case 3: Moving south
                maze[get_index(x, y + 1)] |= CELL_VISITED | CELL_PATH_N; //remove north wall of new cell
                maze[get_index(x, y)] |= CELL_PATH_S; //remove south wall of curent cell
                push((Point){x, y + 1}); //push onto stack, move onto new cell
                break;
            case 3: //Case 4: Moving west
                maze[get_index(x - 1, y)] |= CELL_VISITED | CELL_PATH_E; //remove east wall of new cell
                maze[get_index(x, y)] |= CELL_PATH_W; //remove west wall of current cell
                push((Point){x - 1, y}); //push onto stack, move onto new cell
                break;
            }
            visited_cell++;
        } else {
            //if all cells are visited, backtrack
            pop();
        }   
    }
}

// Drawing function for GTK 4
void draw_maze(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data) {
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_paint(cr);
    
    cairo_set_source_rgb(cr, 1, 1, 1); // White color for paths
    for (int x = 0; x < MAZE_WIDTH; x++) {
        for (int y = 0; y < MAZE_HEIGHT; y++) {
            int index = get_index(x, y);
            if (maze[index] & CELL_VISITED) {
                cairo_rectangle(cr, x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE);
                cairo_fill(cr);
            }
        }
    }
}

-
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
