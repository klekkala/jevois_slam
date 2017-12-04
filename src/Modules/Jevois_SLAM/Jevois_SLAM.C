// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// JeVois Smart Embedded Machine Vision Toolkit - Copyright (C) 2016 by Laurent Itti, the University of Southern
// California (USC), and iLab at USC. See http://iLab.usc.edu and http://jevois.org for information about this project.
//
// This file is part of the JeVois Smart Embedded Machine Vision Toolkit.  This program is free software; you can
// redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software
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
#include "Gaussian.h"
#include "Bilinear.h"
#include <png.h>

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <ctime>
#include <cassert>
#include <jevois/Debug/Log.H>
#include <jevois/Debug/Timer.H>
#include <jevois/Image/RawImageOps.H>
#include <jevois/Core/Module.H>
#include <jevois/Image/RawImageOps.H>
#include <jevois/Types/BoundedBuffer.H>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define IMG_W 640

// icon by Catalin Fertu in cinema at flaticon

//! JeVois sample module
/*! This module is provided as an example of how to create a new standalone module.
    JeVois provides helper scripts and files to assist you in programming new modules, following two basic formats:
    - if you wish to only create a single module that will execute a specific function, or a collection of such modules
      where there is no shared code between the modules (i.e., each module does things that do not relate to the other
      modules), use the skeleton provided by this sample module. Here, all the code for the sample module is compiled
      into a single shared object (.so) file that is loaded by the JeVois engine when the corresponding video output
      format is selected by the host computer.
    - if you are planning to write a collection of modules with some shared algorithms among several of the modules, it
      is better to first create machine vision Components that implement the algorithms that are shared among several of
      your modules. You would then compile all your components into a first shared library (.so) file, and then compile
      each module into its own shared object (.so) file that depends on and automatically loads your shared library file
      when it is selected by the host computer. The jevoisbase library and collection of components and modules is an
      example for how to achieve that, where libjevoisbase.so contains code for Saliency, ObjectRecognition, etc
      components that are used in several modules, and each module's .so file contains only the code specific to that
      module.
    @author Sample Author
    @videomapping YUYV 640 480 28.5 YUYV 640 480 28.5 SampleVendor Jevois_SLAM
    @email sampleemail\@samplecompany.com
    @address 123 First Street, Los Angeles, CA 90012
    @copyright Copyright (C) 2017 by Sample Author
    @mainurl http://samplecompany.com
    @supporturl http://samplecompany.com/support
    @otherurl http://samplecompany.com/about
    @license GPL v3
    @distribution Unrestricted
    @restrictions None */
class Jevois_SLAM : public jevois::Module
{
  public:
    //! Default base class constructor ok
    Jevois_SLAM(std::string const & instance) : jevois::Module(instance), itsRunning(true)
    { }

    //! Virtual destructor for safe inheritance
    virtual ~Jevois_SLAM() { }

    //! Processing function
    virtual void process(jevois::InputFrame && inframe) override
    {
      static int pyramidLevels[16] = {
          640, 480,
          520, 390,
          455, 341,
          370, 277,
          324, 242,
          263, 197,
          230, 172,
          187, 140
      };

      jevois::RawImage inimg = inframe.get(); unsigned int const w = inimg.width, h = inimg.height;
	      cv::Mat src = jevois::rawimage::convertToCvGray(inimg);

	      //itsProcessingTimer.start();
	      // Let camera know we are done processing the input image:
	      inframe.done(); // NOTE: optional here, inframe destructor would call it anyway
	      
      while(itsRunning.load()){
	      //static jevois::Timer itsProcessingTimer("Processing");
	      // Wait for next available camera image:
	      

	      //Using L2 cache smoothing which takes 1ms per 640x480 image
	      uint32_t pyramidHeight = 2239;
	      uint8_t (*im)[IMG_W] = (uint8_t (*)[IMG_W])(src.data); 
	     
	      uint8_t out[pyramidHeight][IMG_W]; 
	      std::vector<uint32_t> points;
	      std::vector<uint32_t> descriptors;

	      bool flag = false;
	      const int vstep = 640; 
	      uint8_t *cam = (uint8_t *)calloc(IMG_W*480, sizeof(uint8_t));
	      uint8_t *cop = (uint8_t *)calloc(IMG_W*480, sizeof(uint8_t));
	      uint32_t total = 0;
	      std::memcpy(cop, im, IMG_W*480*sizeof(uint8_t));
	      uint8_t (*img)[IMG_W] = (uint8_t (*)[IMG_W])malloc(sizeof(uint8_t[2239][IMG_W]));
	      std::memcpy(img, cop, IMG_W*480*sizeof(uint8_t));
	      for (size_t i = 0; i < sizeof(pyramidLevels)/sizeof(*pyramidLevels); i += 2) {

	        pislam::gaussian5x5<vstep>(pyramidLevels[i], pyramidLevels[i+1],
	          (uint8_t (*)[vstep])cop, (uint8_t (*)[vstep])cop);

	        //Alternate downslampling using 7/8 and 13/16
	        if(flag)  
	          pislam::bilinear7_8<vstep>(pyramidLevels[i], pyramidLevels[i+1],
	                  (uint8_t (*)[vstep])cop, (uint8_t (*)[vstep])cam);
	        else{pislam::bilinear13_16<vstep>(pyramidLevels[i], pyramidLevels[i+1],
	                   (uint8_t (*)[vstep])cop, (uint8_t (*)[vstep])cam);
	        }
	        
	        total += pyramidLevels[i+1];
	        
	        if(i <= 12){
	          std::memcpy(img+total, cam, IMG_W*pyramidLevels[i+3]*sizeof(uint8_t));
	          memset(cop, '0', IMG_W*pyramidLevels[i+3]);
	          std::memcpy(cop, cam, 640*pyramidLevels[i+3]*sizeof(uint8_t));
	          memset(cam, '0', IMG_W*pyramidLevels[i+3]);
	        }

	        flag = !flag;
	      }

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
	      pislam::orbCompute<IMG_W, 8>(img, points, descriptors);

	      //Free all the pointers to memory block
	      free(img);
	      free(cam);
	      free(cop);

	      //WRite the fps and processing
	      //std::string const & fpscpu = itsProcessingTimer.stop();
	      //jevois::rawimage::writeText(outimg, fpscpu, 3, 240 - 13, jevois::yuyv::Black);

	      //Started to send points
	      sendSerial(pislam::encode_vecstring(points));
	      //Done with sending points
	      sendSerial(" ");

	      //Starting to send descriptors
	      sendSerial(pislam::encode_vecstring(descriptors));
	      //Done with sending descriptors
	      sendSerial(" ");
	      itsRunning.store(false);
	  }

    }



    // ####################################################################################################
    //! Receive a string from a serial port which contains a user command
    // ####################################################################################################
    void parseSerial(std::string const & str, std::shared_ptr<jevois::UserInterface> s) override
    {
      if (str == "go")
      {
        itsRunning.store(true);
        //sendSerial("START");
      }
     
      else throw std::runtime_error("Unsupported module command");
    }

    std::atomic<bool> itsRunning;
};

// Allow the module to be loaded as a shared object (.so) file:
JEVOIS_REGISTER_MODULE(Jevois_SLAM);