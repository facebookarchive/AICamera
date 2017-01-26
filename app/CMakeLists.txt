cmake_minimum_required(VERSION 3.4.1)

# In order to use some of the headers in Caffe2 we need to include these directories.
include_directories(libs/caffe2)
include_directories(libs/caffe2/third_party/eigen)
include_directories(libs/caffe2/third_party/protobuf/src)
add_subdirectory(libs/caffe2)

add_library( neuralnet-lib src/main/cpp/neuralnet-lib.cpp )

target_link_libraries( neuralnet-lib
                       android
                       -ljnigraphics
                       Caffe2_CPU
                       log )
