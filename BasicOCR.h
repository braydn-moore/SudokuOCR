#ifndef BASIC_OCR_H
#define BASIC_OCR_H

// include files
#include <tesseract/baseapi.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// C++ classes are defined in the header and have constructors like Java but also have destructors as
// C++ doesn't have a garbage collector like Java
class BasicOCR{
    // public methods
    public:
        // Constructor
        BasicOCR();
        // Destructor(Java doesn't have this)
        ~BasicOCR();
        // classify the image
        int classify(cv::Mat img);

    private:
        // method for preprocessing
        void process(cv::Mat& img);
        // api for Optical Character Recognition
        tesseract::TessBaseAPI* ocr;

};

#endif