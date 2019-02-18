// by including the corresponding header this file has access to everything in the header file
// and defines the methods declared in the header
#include "gridFinder.h"

// line structure of two points
struct line{
    cv::Point pt1, pt2;
};

// kernel used to erode and dilate our board
cv::Mat kernel = (cv::Mat_<uchar>(3,3) << 0,1,0,1,1,1,0,1,0);

// check if the board is a valid board
bool isSudoku(cv::Mat image){
    double largest_area=0, area;
    cv::threshold( image, image, 125, 255, cv::THRESH_BINARY ); //Threshold the gray

    std::vector<std::vector<cv::Point> > contours; // Vector for storing contours

    cv::findContours( image, contours, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE ); // Find the contours in the image

    for( size_t i = 0; i< contours.size(); i++ ){ // iterate through each contour.
        area = cv::contourArea( contours[i] );  //  Find the area of contour

        if( area > largest_area ){
            largest_area = area;
        }
    }

    //std::cout<<"Biggest area "<<area<<std::endl;
    return false;
}


// used for debugging to draw a line
void drawLine(cv::Vec2f line, cv::Mat &img, cv::Scalar rgb = CV_RGB(0,0,255)){
    // if the line is on an angle then calculate how to draw the line
    if (line[1]!=0){
        float m = -1/tan(line[1]);
        float c = line[0]/sin(line[1]);
        cv::line(img, cv::Point(0, c), cv::Point(img.size().width, m*img.size().width+c), rgb);
    }
    // otherwise draw a normal line
    else
        cv::line(img, cv::Point(line[0], 0), cv::Point(line[0], img.size().height), rgb);
}

// preprocess the board to improve our outcome
void preprocessing(cv::Mat board, cv::Mat& outer){
    // use a Gaussian blur to make it easier to idenitfy the biggest blob
    cv::GaussianBlur(board, outer, cv::Size(11,11), 0);
    // make a black and white image
    cv::adaptiveThreshold(outer, outer, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 5, 2);
    // invert
    cv::bitwise_not(outer, outer);
    // dilate the image
    dilate(outer, outer, kernel);
}

// find the largest blob in the image
int32_t biggestBlob(cv::Mat& outer){
    // set our variables
    int count = 0;
    int max = -1;
    cv::Point maxPoint;

    // find biggest blob
    int area;
    for (int counter = 0; counter<outer.size().height; counter++){
        uchar* row = outer.ptr(counter);
        for (int counter2 = 0; counter2<outer.size().width; counter2++){
            if (row[counter2]>=128){
                // flood fill the image from the point of where we are in the loop and get the area
                area = cv::floodFill(outer, cv::Point(counter2,counter), CV_RGB(0,0,64));
                // if it is a bigger area then set it as the biggest area found so far
                if(area>max){
                     maxPoint = cv::Point(counter2,counter);
                     max = area;
                 }
            }
        }
    }
    // flood fill with white the point for the biggest blobs
    cv::floodFill(outer, maxPoint, CV_RGB(255,255,255));

    // shift all other blobs to black
    for (int counter = 0; counter<outer.size().height; counter++)
        for (int counter2 = 0; counter2<outer.size().width; counter2++)
            if (outer.ptr(counter)[counter2]==64 && counter2!=maxPoint.x && counter!=maxPoint.y)
               cv::floodFill(outer, cv::Point(counter2, counter), CV_RGB(0,0,0));
    
    // undo the dilate step in the preprocessing so our image is clear to extract numbers
    cv::erode(outer, outer, kernel);
    // return the max area
    return area;
}

struct line calcLine(float rho, float theta, cv::Mat board){
    struct line line1;
    // if the line is approx horizontal then we set our points at the extreme left and right
    if (theta>CV_PI*45/180 && theta<CV_PI*135/180){
            line1.pt1.x = 0;
            line1.pt1.y = rho/sin(theta);

            line1.pt2.y = board.size().width;
            line1.pt2.y = -line1.pt2.x / tan(theta) + rho/sin(theta);
    }
    // otherwise set the line at the top and bottom
    else{
        line1.pt1.y = 0;
        line1.pt1.x = rho/cos(theta);

        line1.pt2.y = board.size().height;
        line1.pt2.x = -line1.pt2.y / tan(theta) + rho/cos(theta);
    }

    return line1;
}

// merge all relatively similar lines so we don't have to iterate through a bunch of lines and get
// more precise location of the board
void mergeCloseLines(cv::Mat board, std::vector<cv::Vec2f>* lines){
    std::vector<cv::Vec2f>::iterator iter;
    for (iter = lines->begin(); iter!=lines->end(); iter++){
        // if rho is zero and theta is -100, these values are impossible so we skip this line
        if ((*iter)[0]==0 && (*iter)[1]==-100) continue;
        
        struct line line1 = calcLine((*iter)[0], (*iter)[1], board);
        int count = 0;
        // iterate over lines again to find similar lines
        for (std::vector<cv::Vec2f>::iterator iter2 = lines->begin(); iter2!=lines->end(); iter2++, count++){
            if (*iter == *iter2) continue;
            // if the line's angles are close then calculate the line of the current line
            if (fabs((*iter2)[0]-(*iter)[0])<20 && fabs((*iter2)[1]-(*iter)[1])<CV_PI*10/180){
                struct line line2 = calcLine((*iter)[0], (*iter2)[1], board);

                // if the endpoints of the lines are close then we can merge them
                if ((pow(line2.pt1.x-line1.pt1.x, 2) + pow(line2.pt1.y-line1.pt1.y, 2)<4096) && (pow(line2.pt2.x-line1.pt2.x, 2) + pow(line1.pt2.y-line2.pt2.y, 2)<4096)){
                    (*iter)[0] = ((*iter)[0]+(*iter2)[0])/2;
                    (*iter)[1] = ((*iter)[1]+(*iter2)[1])/2;

                    // disable the second line(delete without the compiler throwing a bunch of errors)
                    (*iter2)[0] = 0;
                    (*iter2)[1] = -100;
                }
            }
        }
    }

    // get rid of all "impossible lines with and rho of 0 and theta of -100"
    for (std::vector<cv::Vec2f>::iterator iter = lines->begin(); iter!=lines->end();){
        if ((*iter)[0]==0 && (*iter)[1]==-100)
            lines->erase(iter);
        else
            iter++;
    }
    
}

// find the extreme lines of the board eg. the vertical and horizontal lines of the outer grid
void findExtremeLines(cv::Mat& board, std::vector<cv::Vec2f>* lines, cv::Vec2f& topEdge, cv::Vec2f& bottomEdge, cv::Vec2f& leftEdge, cv::Vec2f& rightEdge){
    // initialize our values as the maxs and mins for our next loop
    topEdge = cv::Vec2f(1000,1000);    
    double topYIntercept=100000, topXIntercept=0;
    bottomEdge = cv::Vec2f(-1000,-1000);        
    double bottomYIntercept=0, bottomXIntercept=0;
    leftEdge = cv::Vec2f(1000,1000);    
    double leftXIntercept=100000, leftYIntercept=0;
    rightEdge = cv::Vec2f(-1000,-1000);        
    double rightXIntercept=0, rightYIntercept=0;
    int count = 0;
    // for each line
    for (std::vector<cv::Vec2f>::iterator iter = lines->begin(); iter!=lines->end(); iter++, count++){
        // if the line is nearly vertical
        if((*iter)[1]>CV_PI*80/180 && (*iter)[1]<CV_PI*100/180){
			if((*iter)[0]<topEdge[0])
				topEdge = *iter;

			if((*iter)[0]>bottomEdge[0])
				bottomEdge = *iter;
        }

        // if the line is nearly horizontal
        else if ((*iter)[1]<CV_PI*10/180 || (*iter)[1]>CV_PI*170/180){
            double xIntercept = (*iter)[0]/cos((*iter)[1]);
            double yIntercept = (*iter)[0]/(cos((*iter)[1])*sin((*iter)[1]));
            if(xIntercept>rightXIntercept){
				rightEdge = *iter;
				rightXIntercept = xIntercept;
			} 
			else if(xIntercept<=leftXIntercept){
				leftEdge = *iter;
				leftXIntercept = xIntercept;
            }
        }
    }
}

void getIntersectionValues(double& a, double& b, double& c, struct line edge){
    // get intersection point and modifier of a line us to get the midpoints of the lines to undistort it
    a = edge.pt2.y-edge.pt1.y;
    b = edge.pt1.x-edge.pt2.x;
    c = a*edge.pt1.x + b*edge.pt1.y;
}

cv::Mat undistortImage(cv::Mat original, cv::Vec2f& topEdge, cv::Vec2f& bottomEdge, cv::Vec2f& leftEdge, cv::Vec2f& rightEdge){
    // define the outer board lines
    struct line left, right, top, bottom;
    int width = original.size().width;
    int height = original.size().height;

    // find two points on a line(for the grid) which will later be used to undistort the image

    // if the slope is infinite and calculate the two points using a valid method
    // otherwise use a standard method
    if (leftEdge[1]!=0){
        left.pt1.x = 0;
        left.pt1.y = leftEdge[0]/sin(leftEdge[1]);
        left.pt2.x = width;
        left.pt2.y = -left.pt2.x/tan(leftEdge[1])+left.pt1.y;
    }
    else{
        left.pt1.y = 0;
        left.pt1.x = leftEdge[0]/cos(leftEdge[1]);
        left.pt2.y = height;
        left.pt2.x = left.pt1.x - height*tan(leftEdge[1]);
    }

    // do the same for right
    if (rightEdge[1]!=0){
        right.pt1.x = 0;
        right.pt1.y = rightEdge[0]/sin(rightEdge[1]);
        right.pt2.x = width;
        right.pt2.y = -right.pt2.x/tan(rightEdge[1])+right.pt1.y;
    }
    else{
        right.pt1.y = 0;
        right.pt1.x = rightEdge[0]/cos(rightEdge[1]);
        right.pt2.y = height;
        right.pt2.x = right.pt1.x - height*tan(rightEdge[1]);
    }

    // calculate the top and bottom lines
    bottom.pt1.x=0;    
    bottom.pt1.y=bottomEdge[0]/sin(bottomEdge[1]);
    bottom.pt2.x=width;
    bottom.pt2.y=-bottom.pt2.x/tan(bottomEdge[1]) + bottom.pt1.y;

    top.pt1.x=0;        
    top.pt1.y=topEdge[0]/sin(topEdge[1]);
    top.pt2.x=width;    
    top.pt2.y=-top.pt2.x/tan(topEdge[1]) + top.pt1.y;

    // calculate the intersection values of all the lines for the corners
    double leftA, leftB, leftC, rightA, rightB, rightC, topA, topB, topC, bottomA, bottomB, bottomC;
    getIntersectionValues(leftA, leftB, leftC, left);
    getIntersectionValues(rightA, rightB, rightC, right);
    getIntersectionValues(topA, topB, topC, top);
    getIntersectionValues(bottomA, bottomB, bottomC, bottom);

    // get the difference/modifiers
    double differenceToTopLeft = leftA*topB - leftB*topA;
    double differenceToTopRight = rightA*topB - rightB*topA;
    double differenceToBottomLeft = leftA*bottomB-leftB*bottomA;
    double differenceToBottomRight = rightA*bottomB - rightB*bottomA;

    // calculate the points of the grid
    cv::Point2f topLeft = cv::Point2f((topB*leftC - leftB*topC)/differenceToTopLeft, (leftA*topC - topA*leftC)/differenceToTopLeft);
    cv::Point2f topRight = cv::Point2f((topB*rightC-rightB*topC)/differenceToTopRight, (rightA*topC-topA*rightC)/differenceToTopRight);
    cv::Point2f bottomLeft = cv::Point2f((bottomB*leftC-leftB*bottomC)/differenceToBottomLeft, (leftA*bottomC-bottomA*leftC)/differenceToBottomLeft);
    cv::Point2f bottomRight = cv::Point2f((bottomB*rightC-rightB*bottomC)/differenceToBottomRight, (rightA*bottomC-bottomA*rightC)/differenceToBottomRight);

    // find the longest side length to know what size to crop the image to
    int maxLength = sqrt((double)std::max({(bottomLeft.x-bottomRight.x)*(bottomLeft.x-bottomRight.x) + (bottomLeft.y-bottomRight.y)*(bottomLeft.y-bottomRight.y),
        (topRight.x-bottomRight.x)*(topRight.x-bottomRight.x) + (topRight.y-bottomRight.y)*(topRight.y-bottomRight.y),
        (topRight.x-topLeft.x)*(topRight.x-topLeft.x) + (topRight.y-topLeft.y)*(topRight.y-topLeft.y),
        (bottomLeft.x-topLeft.x)*(bottomLeft.x-topLeft.x) + (bottomLeft.y-topLeft.y)*(bottomLeft.y-topLeft.y)}));

    // initialize the 4 point array for the current source and the dest source with our corners at matching indexes
    cv::Point2f srcGrid[4], destGrid[4];
    srcGrid[0] = topLeft;
    srcGrid[1] = topRight;
    srcGrid[2] = bottomRight;
    srcGrid[3] = bottomLeft;
    destGrid[0] = cv::Point2f(0,0);
    destGrid[1] = cv::Point2f(maxLength-1, 0);
    destGrid[2] = cv::Point2f(maxLength-1, maxLength-1);
    destGrid[3] = cv::Point2f(0, maxLength-1);
    // declare an undistorted image
    cv::Mat undistorted = cv::Mat(cv::Size(maxLength, maxLength), CV_8UC3);
    // undistort the image with all the information we have gathered
    cv::warpPerspective(original, undistorted, cv::getPerspectiveTransform(srcGrid, destGrid), cv::Size(maxLength, maxLength));
    return undistorted;
}

// find lines in the image
std::vector<cv::Vec2f> findLines(cv::Mat& box, cv::Vec2f& topEdge, cv::Vec2f& bottomEdge, cv::Vec2f& leftEdge, cv::Vec2f& rightEdge){
    std::vector<cv::Vec2f> lines;
    // use the hough lines algorithm to find the lines of the grid
    cv::HoughLines(box, lines, 1, CV_PI/180, 200);
    // merge all close lines
    mergeCloseLines(box, &lines);
    // find the extreme lines
    findExtremeLines(box, &lines, topEdge, bottomEdge, leftEdge, rightEdge);
    // draw the lines
    for (int counter = 0; counter<lines.size(); counter++)
        drawLine(lines[counter], box, CV_RGB(0,0,128));
    // return the lines vector
    return lines;
}

// convert between image types of number of rows and form etc.
void convertToCV8UC1(cv::Mat& mat){
    cv::cvtColor(mat,mat, CV_BGR2GRAY);
}

// find the largest contour in a cell(a.k.a the number)
cv::Rect contour(cv::Mat img, int cellSize){
    // find the contours
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(img, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    // if the contour matches an approx. description of the image then return the rectangle around it
    for (size_t counter = 0; counter<contours.size(); counter++){
        std::vector<cv::Point> blob = contours[counter];
        if (cv::contourArea(blob)>((float)cellSize/2)){
            cv::Rect rect = cv::boundingRect(blob);
            if (rect.height>((float)cellSize/2) && rect.height<((float)cellSize*0.8))
                return rect;
        }
    }
    // if no valid number is found return a "null" rectangle which the program will interpret later
    return cv::Rect(0,0,1,1);    
}