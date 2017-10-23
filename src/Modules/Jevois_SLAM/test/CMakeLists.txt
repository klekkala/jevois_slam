cmake_minimum_required(VERSION 2.8)
project( DisplayImage )
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} -Wall -mfpu=neon")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -mfpu=neon")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -mfpu=neon")

find_package( OpenCV REQUIRED )
add_executable( DisplayImage raspi_test.cpp )
target_link_libraries( DisplayImage ${OpenCV_LIBS} )
