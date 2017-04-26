cmake_minimum_required(VERSION 3.4.1)

add_library(
             native-lib
             SHARED
             src/main/cpp/native-lib.cpp
             )
find_library(
          android-lib
          android
          )

include(AndroidNdkModules)
android_ndk_import_module_cpufeatures()

add_library(
    caffe2
    STATIC
    IMPORTED
    )
set_target_properties(
    caffe2
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_CURRENT_LIST_DIR}/src/main/jniLibs/${ANDROID_ABI}/libcaffe2.a
    )

add_library(
    glog
    SHARED
    IMPORTED
    )
set_target_properties(
    glog
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_CURRENT_LIST_DIR}/src/main/jniLibs/${ANDROID_ABI}/libglog.so
    )

add_library(
    protobuf
    SHARED
    IMPORTED
    )
set_target_properties(
    protobuf
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_CURRENT_LIST_DIR}/src/main/jniLibs/${ANDROID_ABI}/libprotobuf.a
    )

add_library(
    NNPACK
    STATIC
    IMPORTED
    )
set_target_properties(
    NNPACK
    PROPERTIES IMPORTED_LOCATION
    ${CMAKE_CURRENT_LIST_DIR}/src/main/jniLibs/${ANDROID_ABI}/libNNPACK.a
    )

include_directories( src/main/cpp )

find_library(
     log-lib
     log
     )

target_link_libraries(
                       native-lib
                       -Wl,--whole-archive
                       caffe2
                       -Wl,--no-whole-archive
                       NNPACK
                       glog
                       protobuf
                       cpufeatures
                       ${log-lib}
                       ${android-lib})