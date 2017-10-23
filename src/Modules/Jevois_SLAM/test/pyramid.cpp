#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace cv;


/// Global variables
Mat src, dst, tmp, Image;
char* window_name = "Pyramids Demo";


/**
 * @function main
 */
int main( int argc, char** argv )
{
  /// General instructions
  printf( "\n Zoom In-Out demo  \n " );
  printf( "------------------ \n" );
  printf( " * [u] -> Zoom in  \n" );
  printf( " * [d] -> Zoom out \n" );
  printf( " * [ESC] -> Close program \n \n" );

  /// Test image - Make sure it s divisible by 2^{n}
  src = imread( "originalLeopard.jpg");
  cvtColor(src,src,CV_RGB2GRAY);

  if( !src.data )
    { printf(" No data! -- Exiting the program \n");
      return -1; }

  tmp = src;
  dst = tmp;

  /// Create window
  namedWindow( window_name, CV_WINDOW_AUTOSIZE );
  imshow( window_name, dst );

  /// Loop
  Image = src;
  int abs_height = src.rows;
  int abs_width = src.cols;
  static int pyramidLevels[16] = {
        640, 480,
        533, 400,
        444, 333,
        370, 278,
        309, 231,
        257, 193,
        214, 161,
        179, 134
      };
  for(int i = 2; i< 16; i += 2)
      {
        //Initialize a new cv::Mat
        cv::Mat dst(pyramidLevels[i+1], pyramidLevels[i], CV_8UC1);
        cv::GaussianBlur(src, src, cv::Size(7, 7), 0, 0);
        cv::resize(src, dst, dst.size(), 0, 0);
        cv::copyMakeBorder(dst, dst, 0, 0, 0, abs_width-dst.cols, cv::BORDER_CONSTANT, 0);
        cv::vconcat(Image, dst, Image);
        src = dst;
      }


  imwrite("output.png", Image);
  return 0;
}