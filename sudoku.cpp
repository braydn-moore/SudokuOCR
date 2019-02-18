#include "sudoku.h"

// small function to get the color of the quadrants when drawing the grid
int getColor(int row, int col, int cellSize){
    return (col/(cellSize*2))+(row/cellSize)*3+1;
}

// draw the numbers in the grid
void SudokuGame::drawNumbers(WINDOW* win, int cellSize, bool cursor){
    // move the cursor to the start of the window
    wmove(win, 0,0);
    // reset the cursor color to white
    wattron(win, COLOR_PAIR(1));
    // iterate over each cell in the grid
    for (int counter = 0; counter<9; counter++)
        for (int counter2 = 0; counter2<9; counter2++){
            // move to the center of the cell
            wmove(win, counter*cellSize+ceil((float)cellSize/2), counter2*(cellSize*2)+ceil((float)cellSize/2));
            
            // if the cell is where the cursor is and we are in edit mode then invert the color to indicate the cursor
            if (counter == selY && counter2==selX && cursor) wattron(win, A_REVERSE);
            // otherwise reset the inverted value
            else wattroff(win, A_REVERSE);

            // if the value in the array is zero then we don't print a number
            if (board[counter][counter2]==0){wprintw(win, " ");};
            // otherwise print the number
            if(board[counter][counter2]!=0)wprintw(win, std::to_string(board[counter][counter2]).c_str());
        }
}

// draw the sudoku grid
void SudokuGame::drawGrid(WINDOW* win, int rows, int cols, int sideLength){
    // get the spacing of the columns and rows given the side length
    int colsSpacing = (sideLength*2)/cols;
    int rowSpacing = sideLength/rows;
    // get the spacing in strings for rows and columns
    std::string colSpace = std::string(colsSpacing-1, ' ');
    std::string rowSpace = std::string(rowSpacing-1, '\n');
    // iterate over every row and column
    for (int row = 0; row<sideLength; row++){
        for (int col = 0; col<sideLength*2; col++){
            // set the color to the color of the quadrant
            wattron(win, COLOR_PAIR(getColor(row, col, sideLength/3)));
            // print a blank cell if we are at the beginning or end of the grid
            if (row==0 || col == 0 || row==sideLength || col==sideLength*2) wprintw(win, " ");
            // if we need to print a row then print it
            else if (row%rowSpacing==0) wprintw(win, "-");
            // otherwise if we need to print a column divider then print it
            else if (col%colsSpacing == 0) wprintw(win, "|");
            // otherwise just print a blank cell
            else wprintw(win, " ");
        }
    } 
}

// get the next square to try to solve for
void SudokuGame::getNextEmptySquare(int& row, int& col){
    // iterate over cells from left to right, up to down, and if one is empty return it
    while (board[row][col]!=0 && row<9){
        col++;
        if (col>8){
            col = 0;
            row++;
        }
    }
    if(row==9) row--;
}

// check if the board is solved
bool SudokuGame::solved(){
    // for each item in the board
    for (auto& arr : board)
        for (auto& num : arr)
            // if there is an unsolved cell then it is not solved
            if (num==0) return false;
    // otherwise it is
    return true;
}


// get all possible inputs
std::vector<int> SudokuGame::possibleInputs(int row, int col){
    // all possible inputs for a cell
    std::vector<int> possible = {1,2,3,4,5,6,7,8,9};
    // check every cell in the column and row of the current cell and remove the value
    // from the array that is in each cell if they are not 0
    // this follows the sudoku rule that there cannot be 2+ of the same digit in the same row and column
    for (int counter = 0; counter<9; counter++){
        if (board[row][counter]!=0 && counter!=col && std::find(possible.begin(), possible.end(), board[row][counter])!=possible.end()) possible.erase(std::remove(possible.begin(), possible.end(), board[row][counter]), possible.end());
        if (board[counter][col]!=0 && counter!=row && std::find(possible.begin(), possible.end(), board[counter][col])!=possible.end()) possible.erase(std::remove(possible.begin(), possible.end(), board[counter][col]), possible.end());
    }
    return possible;
}

// recursive solve function
bool SudokuGame::solve(WINDOW* win, int cellSize, int row, int col){
    // if the board is solved then return true
    if (solved()) return true;
    // get the next empty square to solve for
    getNextEmptySquare(row, col);

    // for all values in possible values
    for (auto& possibleValue : possibleInputs(row, col)){
        // set the board position to the value
        board[row][col] = possibleValue;
        // if we are animating then pause
        if (ANIMATION){
            drawNumbers(win, cellSize, false);
            wrefresh(win);
            refresh();
            std::this_thread::sleep_for(ANIMATION_WAIT);
        }
        // attempt to solve from this point out using the current value
        if (solve(win, cellSize, row, col)) return true;
        // if we get back set the cell to 0 so we can change it again
        board[row][col] = 0;
    }
    // return false if the board cannot be solved
    return false;
}

void SudokuGame::main(){
    // initialize the screen
    initscr();
    // prevent echo
    noecho();
    cbreak();
    // init colors
    start_color();
    // put the cursor in the top left corner
    curs_set(0);
    // initialize the colors
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_YELLOW, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(8, COLOR_BLUE, COLOR_BLACK);
    init_pair(9, COLOR_RED, COLOR_BLACK);
    
    // get the measurements for the board
    int maxX, maxY;
    getmaxyx(stdscr, maxY, maxX);
    int gridSideLength = min(maxX, maxY);
    gridSideLength-=gridSideLength%9;
    
    // create the window
    WINDOW* main = newwin(gridSideLength, gridSideLength*2, 2, 2);
    // allow for the arrow keys to be registered as inputs
    keypad(main, true);
    int input;
    // if the window size is too small then exit
    if (((float)gridSideLength/9)<2) exit(1);
    // draw the grid
    drawGrid(main, 9, 9, gridSideLength);
    
    // main loop
    while (true){
        // draw the numbers of the board
        drawNumbers(main, gridSideLength/9, true);
        // refresh the screen
        refresh();
        wrefresh(main);
        // get user input
        input = wgetch(main);

        // change position and board value given valid user input
        switch(input){
            case KEY_UP:
                selY--;
                if (selY==-1) selY=8;
                break;
            case KEY_DOWN:
                selY++;
                if (selY==9) selY = 0;
                break;
            case KEY_LEFT:
                selX--;
                if(selX==-1) selX = 8;
                break;
            case KEY_RIGHT:
                selX++;
                if (selX==9) selX = 0;
                break;
            case KEY_BACKSPACE:
            case KEY_DC:
            case 127:
                board[selY][selX] = 0;
                break;
            default:
                if (isdigit(input)) board[selY][selX] = input-'0';
                break;
        }

        // if the user signals to solve exit and start solving
        if (input == 's')
            break;
    }
    // solve the board
    solve(main, gridSideLength/9, 0, 0);
    // draw our answer
    drawNumbers(main, gridSideLength/9, false);
    // refresh the window
    wrefresh(main);
    refresh();
    // press any key to exit
    getch();
    // clean up
    endwin();
}