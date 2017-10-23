// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Foundation, version 2.  This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
// License for more details.  You should have received a copy of the GNU General Public License along with this program;
// if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
//
// Contact information: Laurent Itti - 3641 Watt Way, HNB-07A - Los Angeles, CA 90089-2520 - USA.
// Tel: +1 213 740 3527 - itti@pollux.usc.edu - http://iLab.usc.edu - http://jevois.org
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*! \file */

#include "Fast.h"
#include "Util.h"
#include "Orb.h"

#include <png.h>

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <ctime>
#include <cassert>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/opencv.hpp"
#define IMG_W 640

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
 
uint8_t *read_frame(cv::Mat src, uint32_t *width, uint32_t *height);
int main()
    {

      /*// Wait for next available camera image:
      jevois::RawImage const inimg = inframe.get(true);

      // We only support YUYV pixels in this example, any resolution:
      inimg.require("input", inimg.width, inimg.height, V4L2_PIX_FMT_YUYV);
      
      // Let camera know we are done processing the input image:
      inframe.done(); // NOTE: optional here, inframe destructor would call it anyway

      // Wait for an image from our gadget driver into which we will put our results:
      jevois::RawImage outimg = outframe.get();

      // Enforce that the input and output formats and image sizes match:
      outimg.require("output", inimg.width, inimg.height, inimg.fmt);
      
      //Copy the pixel data over
      memcpy(outimg.pixelsw<void>(), inimg.pixels<void>(), std::min(inimg.buf->length(), outimg.buf->length()));
      // Print a text message:
      //jevois::rawimage::writeText(outimg, "Hello JeVois!", 100, 230, jevois::yuyv::White, jevois::rawimage::Font20x38);
      
      //Currently using opencv for pyramid generation
      //Will be replaced by L2 cache smoothing which takes 1ms per 640x480 image*/
      cv::Mat src = cv::imread("test.jpg", cv::IMREAD_GRAYSCALE);
      cv::Mat Image = src;
      int abs_width = src.cols;
      for(int i = 2; i< 16; i += 2)
      {
        //Initialize a new cv::Mat
        cv::Mat dst(pyramidLevels[i+1], pyramidLevels[i], CV_8UC1);
	std::cout << dst.rows << " " << dst.cols << std::endl;
        cv::GaussianBlur(src, src, cv::Size(7, 7), 0, 0);
        cv::resize(src, dst, dst.size(), 0, 0);
        cv::copyMakeBorder(dst, dst, 0, 0, 0, abs_width-dst.cols, cv::BORDER_CONSTANT, 0);
        cv::vconcat(Image, dst, Image);
        src = dst;
      }

      uint32_t pyramidHeight = 0;
      for (size_t i = 1; i < sizeof(pyramidLevels)/sizeof(*pyramidLevels); i += 2) {
        pyramidHeight += pyramidLevels[i];
      }
      //Feed in image pyramid to ORB featurure detector
      uint32_t width, height;
      std::cout << " df " << Image.size().width << std::endl;
      uint8_t (*img)[IMG_W] = (uint8_t (*)[IMG_W])read_frame(Image, &width, &height);
      std::cout << width << " " << height << " " << std::endl;
      //memcpy(img, Image.data, Image.total()*Image.elemSize()*sizeof(uint8_t));
      /*int j = 0;
      int index = 0;
      for(int i = 0; i<Image.cols; i++){
        img[index++] = Image.at<uchar>(i, j);
        if(i == Image.cols && j != Image.rows){
          j++;
          i = 0;
        }
      }*/


      uint8_t out[pyramidHeight][IMG_W];
      std::vector<uint32_t> points;
      std::vector<uint32_t> descriptors;
      
      //Start filling in individual levels of a pyramid to compute FAST
      uint32_t pyramidRow = 0;
      for (size_t i = 0; i < sizeof(pyramidLevels)/sizeof(*pyramidLevels); i += 2) {
        uint32_t levelWidth = pyramidLevels[i];
        uint32_t levelHeight = pyramidLevels[i+1];

        uint8_t (*imgPtr)[IMG_W] = &img[pyramidRow];
        uint8_t (*outPtr)[IMG_W] = &out[pyramidRow];

        pislam::fastDetect<IMG_W, 16>(levelWidth, levelHeight, imgPtr, outPtr, 20);
        pislam::fastScoreHarris<IMG_W, 16>(levelWidth, levelHeight, imgPtr, 1 << 15, outPtr);

        size_t oldSize = points.size();
        pislam::fastExtract<IMG_W, 16>(levelWidth, levelHeight, outPtr, points);

        // Adjust y coordinate to match position in image pyramid.
        for (auto p = points.begin() + oldSize; p < points.end(); ++ p) {
          uint32_t x = pislam::decodeFastX(*p);
          uint32_t y = pislam::decodeFastY(*p) + pyramidRow;
          uint32_t score = pislam::decodeFastScore(*p);
          *p = pislam::encodeFast(score, x, y);
        }

        pyramidRow += levelHeight;
      }
      //Process the image
      pislam::orbCompute<IMG_W, 8>(img, points, descriptors);

      for (uint32_t point: points) {
        uint32_t x = pislam::decodeFastX(point);
        uint32_t y = pislam::decodeFastY(point);
        //jevois::rawimage::drawCircle(outimg, x, y, 1, 1, 1);
      }

      //jevois::rawimage::drawCircle(outimg, 400, 400, 1, 1, 1);
      // Send the output image with our processing results to the host over USB:
      //cv::imwrite("output.png",src); // NOTE: optional here, outframe destructor would call it anyway
      std::cout << points.size() << std::endl;
      return 0;
    }


uint8_t *read_frame(cv::Mat src, uint32_t *width, uint32_t *height){
	*width = src.cols;
	std::cout << " size is " << src.cols << std::endl;
	*height = src.rows;
	uint8_t *out = (uint8_t *)malloc((*width)*(*height));
	for (uint32_t y = 0; y < *height; y += 1) {
		for (uint32_t x = 0; x < *width; x += 1) {
			out[y*(*width)+x] = src.at<uchar>(y, x);
			}
			//free(row_pointers[y]);
		}
	      //free(row_pointers);
	        return out;
}
// Allow the module to be loaded as a shared object (.so) file:
