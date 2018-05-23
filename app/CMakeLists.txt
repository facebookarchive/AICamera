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

include_directories( src/main/cpp )
include_directories( src/pytorch )
include_directories( src/pytorch/build_android )
include_directories( src/pytorch/third_party/protobuf/src )

find_library(
     log-lib
     log
     )

target_link_libraries(
    native-lib
    -Wl,--whole-archive
    ${CMAKE_CURRENT_LIST_DIR}/src/pytorch/build_android/lib/libcaffe2.a
    ${CMAKE_CURRENT_LIST_DIR}/src/pytorch/build_android/lib/libcaffe2_protos.a
    ${CMAKE_CURRENT_LIST_DIR}/src/pytorch/build_android/lib/libcpuinfo.a
    ${CMAKE_CURRENT_LIST_DIR}/src/pytorch/build_android/lib/libnnpack.a
    ${CMAKE_CURRENT_LIST_DIR}/src/pytorch/build_android/lib/libnnpack_reference_layers.a
    ${CMAKE_CURRENT_LIST_DIR}/src/pytorch/build_android/lib/libnomnigraph.a
    ${CMAKE_CURRENT_LIST_DIR}/src/pytorch/build_android/lib/libonnx.a
    ${CMAKE_CURRENT_LIST_DIR}/src/pytorch/build_android/lib/libonnx_proto.a
    ${CMAKE_CURRENT_LIST_DIR}/src/pytorch/build_android/lib/libprotobuf.a
    # ${CMAKE_CURRENT_LIST_DIR}/src/pytorch/build_android/lib/libprotobuf-lite.a
    # ${CMAKE_CURRENT_LIST_DIR}/src/pytorch/build_android/lib/libpthreadpool.a
    -Wl,--no-whole-archive
    ${log-lib}
    ${android-lib}
)