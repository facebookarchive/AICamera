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

include_directories( src/main/cpp )

set(PYTORCH_PATH src/main/cpp/pytorch)
execute_process(
    COMMAND bash scripts/build_host_protoc.sh
    WORKING_DIRECTORY ${PYTORCH_PATH}
    RESULT_VARIABLE build_host_protoc_result
)
if (NOT build_host_protoc_result EQUAL 0)
    message(FATAL_ERROR "Cannot build host protoc. ${build_host_protoc_result}")
endif()
get_filename_component(CAFFE2_CUSTOM_PROTOC_EXECUTABLE ${PYTORCH_PATH}/build_host_protoc/bin/protoc ABSOLUTE)
set(BUILD_TEST OFF CACHE BOOL "" FORCE)
set(BUILD_BINARY OFF CACHE BOOL "" FORCE)
set(BUILD_PYTHON OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(USE_CUDA OFF CACHE BOOL "" FORCE)
set(USE_GFLAGS OFF CACHE BOOL "" FORCE)
set(USE_OPENCV OFF CACHE BOOL "" FORCE)
set(USE_LMDB OFF CACHE BOOL "" FORCE)
set(USE_LEVELDB OFF CACHE BOOL "" FORCE)
set(USE_MPI OFF CACHE BOOL "" FORCE)
set(USE_OPENMP OFF CACHE BOOL "" FORCE)
set(USE_MOBILE_OPENGL OFF CACHE BOOL "" FORCE)
add_subdirectory( ${PYTORCH_PATH} )
include_directories( $<TARGET_PROPERTY:caffe2,INCLUDE_DIRECTORIES> )

find_library(
    log-lib
    log
)

target_link_libraries(
    native-lib
    -Wl,--whole-archive
    caffe2
    -Wl,--no-whole-archive
    ${log-lib}
    ${android-lib}
)