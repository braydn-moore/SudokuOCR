
// C++ uses headers to declare methods and declare/define fields for classes and source code
// the include guards ensure that functions aren't declared twice
#ifndef GRID_FINDER_H
#define GRID_FINDER_H

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <vector>
#include <iostream>

// used for debugging and drawing a line
void drawLine(cv::Vec2f line, cv::Mat &img, cv::Scalar rgb);
// cleans up the object
void preprocessing(cv::Mat board, cv::Mat& outer);
// Finds the biggest "blob"(the board) in the image
int biggestBlob(cv::Mat& outer);
// calculates the a line given the rho, theta and the board
struct line calcLine(float rho, float theta, cv::Mat board);
// merges lines that are relatively similar
void mergeCloseLines(cv::Mat board, std::vector<cv::Vec2f>* lines);
// finds the lines that are the most extreme(most vertical and horizontal)
void findExtremeLines(cv::Mat& board, std::vector<cv::Vec2f>* lines, cv::Vec2f& topEdge, cv::Vec2f& bottomEdge, cv::Vec2f& leftEdge, cv::Vec2f& rightEdge);
// gets the intersection of various values from an edge
void getIntersectionValues(double& a, double& b, double& c, struct line edge);
// undistort the image given the corners of the board
cv::Mat undistortImage(cv::Mat original, cv::Vec2f& topEdge, cv::Vec2f& bottomEdge, cv::Vec2f& leftEdge, cv::Vec2f& rightEdge);
// get the lines of the sudoku board
std::vector<cv::Vec2f> findLines(cv::Mat& box, cv::Vec2f& topEdge, cv::Vec2f& bottomEdge, cv::Vec2f& leftEdge, cv::Vec2f& rightEdge);
// check if the board is an actual sudoku board
bool isSudoku(cv::Mat image);
// contour the cell to crop to the number in the cell and reduce noise
cv::Rect contour(cv::Mat img, int cellSize);
// convert to CV_8UC1 image type for compatibility
void convertToCV8UC1(cv::Mat& mat);


#endif