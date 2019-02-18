#include "BasicOCR.h"

// In this case the double colon means we are defining a function of the class from the header
BasicOCR::BasicOCR(){
    // create the object
    ocr = new tesseract::TessBaseAPI();
    // tell the api the image will only have one character
    ocr->SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
    // only allow 1-9 to be returned
    ocr->SetVariable("tessedit_char_whitelist","123456789");
    // use the english data from the local tesdata folder
    ocr->Init("./tessdata", "eng", tesseract::OEM_DEFAULT);
}

BasicOCR::~BasicOCR(){
    // cleanup our ocr object
    delete ocr;
}

void BasicOCR::process(cv::Mat& img){
    // add a border and resize the image for optimal image recognition
    cv::copyMakeBorder(img, img, 10,10,10,10,cv::BORDER_CONSTANT | cv::BORDER_ISOLATED, cv::Scalar(0,0,0));
    cv::resize(img, img, cv::Size(75,75));
}

int BasicOCR::classify(cv::Mat img){
    // preprocess the image
    process(img);
    // read the image into the ocr object
    ocr->SetImage((uchar*)img.data, img.cols, img.rows, 1, img.cols);
    // return the classified text as an integer
    return atoi(ocr->GetUTF8Text());
}