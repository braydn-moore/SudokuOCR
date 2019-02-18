cmake_minimum_required(VERSION 3.10)
project(Stage2)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

find_package( OpenCV REQUIRED )
find_package(Curses REQUIRED)
find_package( PkgConfig REQUIRED)

pkg_search_module( TESSERACT REQUIRED tesseract )

pkg_search_module( LEPTONICA REQUIRED lept )

include_directories( ${TESSERACT_INCLUDE_DIRS} )

include_directories( ${LEPTONICA_INCLUDE_DIRS} )

link_directories( ${TESSERACT_LIBRARY_DIRS} )

link_directories( ${LEPTONICA_LIBRARY_DIRS} )


include_directories(${CURSES_INCLUDE_DIR})
include_directories(${Tesseract_INCLUDE_DIRS})

add_executable(Stage2
        tessdata/configs/ambigs.train
        tessdata/configs/api_config
        tessdata/configs/bigram
        tessdata/configs/box.train
        tessdata/configs/box.train.stderr
        tessdata/configs/digits
        tessdata/configs/hocr
        tessdata/configs/inter
        tessdata/configs/kannada
        tessdata/configs/linebox
        tessdata/configs/logfile
        tessdata/configs/lstm.train
        tessdata/configs/makebox
        tessdata/configs/pdf
        tessdata/configs/quiet
        tessdata/configs/rebox
        tessdata/configs/strokewidth
        tessdata/configs/tsv
        tessdata/configs/txt
        tessdata/configs/unlv
        tessdata/tessconfigs/batch
        tessdata/tessconfigs/batch.nochop
        tessdata/tessconfigs/matdemo
        tessdata/tessconfigs/msdemo
        tessdata/tessconfigs/nobatch
        tessdata/tessconfigs/segdemo
        tessdata/eng.traineddata
        tessdata/pdf.ttf
        BasicOCR.cpp
        BasicOCR.h
        gridFinder.cpp
        gridFinder.h
        main.cpp
        sudoku.cpp
        sudoku.h
        test.jpg
        test2.jpg
        test3.jpg
        test4.png)


target_link_libraries( Stage2 ${OpenCV_LIBS} ${CURSES_LIBRARIES} ${LEPTONICA_LIBRARIES} ${TESSERACT_LIBRARIES})-Duser.name=x3vikan
-Duser.name=x3vikan
-Duser.name=x3vikan
