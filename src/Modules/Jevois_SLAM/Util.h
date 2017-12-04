#ifndef PISLAM_UTIL_H_
#define PISLAM_UTIL_H_

#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <iostream>
#include <cstdint>

namespace pislam {


std::string encode_vecstring(std::vector<uint32_t> vec){

	std::ostringstream oss;

	if (!vec.empty()){
	// Convert all but the last element to avoid a trailing ","
		std::copy(vec.begin(), vec.end()-1,
			std::ostream_iterator<int>(oss, ","));
	// Now add the last element with no delimiter
		oss << vec.back();
	}
	return oss.str();
}

/*uint8_t *read_frame(cv::Mat src, uint32_t *width, uint32_t *height){
  *width = src.cols;
  std::cout << " size is " << src.cols << std::endl;
  *height = src.rows;
  uint8_t *out = (uint8_t *)malloc((*width)*(*height));
  for (uint32_t y = 0; y < *height; y += 1) {
  	for (uint32_t x = 0; x < *width; x += 1) {
      out[y*(*width)+x] = src.at<uchar>(y, x);
  	}
  }
  return out;
}*/


static inline uint32_t encodeFast(uint32_t score, uint32_t x, uint32_t y) {
  return (score << 24) | (x << 12) | y;
}

static inline uint32_t rencodeFastScore(uint32_t score, uint32_t encoded) {
  return (score << 24) | (encoded & 0xffffff);
}

static inline uint32_t decodeFastX(uint32_t encoded) {
  return (encoded >> 12) & 0xfff;
}

static inline uint32_t decodeFastY(uint32_t encoded) {
  return encoded & 0xfff;
}

static inline uint32_t decodeFastScore(uint32_t encoded) {
  return encoded >> 24;
}

} /* namespace pislam */
#endif /* PISLAM_FAST_H_ */
