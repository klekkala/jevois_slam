#include "Fast.h"
#include "Util.h"
#include "Orb.h"

#include <png.h>

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <ctime>
#include <cassert>
#include <jevois/Core/Module.H>
#include <jevois/Image/RawImageOps.H>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>



using namespace cv;

int main(){
// Capture the Image from the webcam
VideoCapture cap(0);

// Get the frame
Mat save_img; cap >> save_img;

if(save_img.empty())
{
  std::cerr << "Something is wrong with the webcam, could not get frame." << std::endl;
}
// Save the frame into a file
imwrite("test.jpg", save_img); // A JPG FILE IS BEING SAVED

return 0;
}
