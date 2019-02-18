#ifndef SUDOKU_H
#define SUDOKU_H
#include <ncurses.h>
#include <string>
#include <math.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>

// min function macro
#define min(a,b) (((a)<(b))? a:b)
// time to sleep for the animation time 
#define ANIMATION_WAIT std::chrono::milliseconds(15)
// should the grid be animated
#define ANIMATION true

class SudokuGame{
    // public methods
    public:
        // C++ has operator overloading of objects, which allows you to run custom code when operators(eg. +,-,*,/) are used on objects
        // operater overloading of brackets to access the array
        // note: in c++ the [] overloading doesn't work for 2d arrays easy so I had to use ()
        int& operator()(int r, int c){
            return board[r][c]; 
        }

        // main function
        void main();
    
    private:
        // solve the board
        bool solve(WINDOW* win, int cellSize, int row, int col);
        // check if the board is solved
        bool solved();
        // get the next empty square to solve for
        void getNextEmptySquare(int& row, int& col);
        // return a vector of all possible values that can be put in the cell based on
        // previous assumed values in accordance with sudoku rules
        std::vector<int> possibleInputs(int row, int col);
        // C++ allows for variables to be passed as a pointer which instead of passing the value of the variable,
        // passes the value of the memory address of where the value is stored
        
        // draw the sudoku grid
        void drawGrid(WINDOW* win, int rows, int cols, int sideLength);
        // draw the numbers in the grid
        void drawNumbers(WINDOW* win, int cellSize, bool cursor);
        // the sudoku board
        int board[9][9];
        // cursor position for the user to edit the board
        int selX = 0, selY = 0;
};

#endif