#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "gridFinder.h"
#include "BasicOCR.h"
#include "sudoku.h"
#include <math.h>

// In C++ functions, the & symbol allows parameters to be passed by reference
// this allows for changes to the variable in the function to be made to the variable in the scope where
// the function is called. This is useful when more than 1 value needs to be returned

// gets the sudoku cropped image grid
bool getSudokuGrid(cv::Mat& sudoku){
    // C++ has namespaces so you can have multiple functions with the same name but in different name spaces
    // the double colon means a function or value in said namespace
    
    // creates the main image we are going to use
    cv::Mat outer = cv::Mat(sudoku.size(), CV_8UC1);
    // declare the 2d vectors we are going to use for the corners of the board
    cv::Vec2f topEdge, bottomEdge, leftEdge, rightEdge;
    // preprocess/pretiffy our image
    preprocessing(sudoku, outer);
    // find the biggest blob
    biggestBlob(outer);
    // find the lines of the board
    // Vectors in C++ are the equivilant of Arraylists in Java
    std::vector<cv::Vec2f> lines = findLines(outer, topEdge, bottomEdge, leftEdge, rightEdge);
    
    // if there aren't enough lines then return false
    if (lines.size()<8)
        return false;

    // set the image to the undistorted cropped image of the board
    sudoku = undistortImage(sudoku, topEdge, bottomEdge, leftEdge, rightEdge);
    // got the board
    return true;
}

// get the board input from the video camera
cv::Mat getInput(cv::VideoCapture* cap){
    // if the camera is not open then leave
    // -> must be used to call a function or access a field for a pointer to an object
    if (!cap->isOpened())
        exit(1);
    
    // declare our images
    cv::Mat frame, img;
    // if we have a valid board from the frame
    bool gotBoard;
    // infinite loop
    for (;;){
        // read the current frame
        cap->read(frame);
        // clone it(objects are pass by reference so we need to clone to avoid both images being changed);
        img = frame.clone();
        // convert the image to the correct number of image streams and inputs
        convertToCV8UC1(img);
        // try to get the board
        gotBoard = getSudokuGrid(img);
        if (gotBoard)
            cv::imshow("Board", img);
        else
            // get rid of the window from last time
            cv::destroyWindow("Board");
        cv::imshow("Press Space when the correct board is displayed", frame);
        int key = cv::waitKey(30);
        // if the user presses the escape key then exit
        if (key == 27)
            exit(0);
        // if the user presses the space key then break and continue with the program
        else if (key == 32 && gotBoard)
            break;
    }
    // get rid of all the windows
    cv::destroyAllWindows();
    // clean up and return the image
    delete cap;
    return img;
}

// get the input from a local photo
cv::Mat getInput(std::string path){
    // read and return the image
    return cv::imread(path, CV_8UC1);
}


// C++ allows for command line arguments stored in argv which we can use later in the program
int main(int argc, char ** argv){
    // create all of our objects we need
    // The new keyword in C++ returns a pointer to an object
    SudokuGame* game = new SudokuGame();
    BasicOCR* ocr = new BasicOCR();
    //cv::VideoCapture* cap = new cv::VideoCapture(0);
    // check if we have valid input
    //if (argc<2){
    //    std::cout<<"You need to provide a picture for input"<<std::endl;
    //    exit(1);
    //}
    // get our input
    cv::Mat img = getInput("test.jpg");
    
    // get the board
    getSudokuGrid(img);
    // threshold so we are in black and white
    cv::Mat undistortedAdjusted = img.clone();
    cv::adaptiveThreshold(img, undistortedAdjusted, 255, CV_ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY_INV, 101, 1);
    // get the cell size
    int cellSize = ceil((double)(img.size().width/9));
    // declare and define the current cell image variable
    cv::Mat currentCell = cv::Mat(cellSize, cellSize, CV_8UC1);

    // for each cell in the board
    for (int counter = 0; counter<9; counter++){
        for (int counter2 = 0; counter2<9; counter2++){
            
            // copy from undistortedAdjusted to the current cell value
            for (int y = 0; y<cellSize && counter*cellSize+y<undistortedAdjusted.rows; y++)
                for (int x = 0; x<cellSize && counter2*cellSize+x<undistortedAdjusted.cols; x++)    
                    currentCell.ptr(y)[x] = undistortedAdjusted.at<uchar>(counter*cellSize+y, counter2*cellSize+x);
            
            // moments are used to check distribution and a bunch of other seriously advanced math
            // I just use them to check if I have an empty image
            cv::Moments moment = cv::moments(currentCell, true);
            cv::Rect rect;
            // if the distribution is greater than 1/5 of the total area then it is an actual number we need to determine
            if (moment.m00>currentCell.rows*currentCell.cols/5 && (rect = contour(currentCell.clone(), cellSize)).area()!=1 ){
                // crop any excess board lines we don't need by contouring the image to find the central focus a.k.a the number
                cv::Mat next = currentCell(rect);
                // classify and read the number into the game object's board array
                (*game)(counter,counter2) = ocr->classify(next);
            }
            // if no number is found then set the value to nothing
            else
                (*game)(counter,counter2) = 0;
        }
    }

    // pass control to the sudoku game
    game->main();
    // clean up
    delete game;
    delete ocr;
    return 0;
}
