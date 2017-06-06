cmake_minimum_required(VERSION 3.4.1)

set(distribution_DIR ${CMAKE_SOURCE_DIR}/../../../distribution)


add_library(libavcodec STATIC IMPORTED)
add_library(libavdevice STATIC IMPORTED)
add_library(libavfilter STATIC IMPORTED)
add_library(libavformat STATIC IMPORTED)
add_library(libavutil STATIC IMPORTED)
add_library(libswresample STATIC IMPORTED)
add_library(libswscale STATIC IMPORTED)

set_target_properties(libavcodec PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/lib/${ANDROID_ABI}/libavcodec.a)
set_target_properties(libavdevice PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/lib/${ANDROID_ABI}/libavdevice.a)
set_target_properties(libavfilter PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/lib/${ANDROID_ABI}/libavfilter.a)
set_target_properties(libavformat PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/lib/${ANDROID_ABI}/libavformat.a)
set_target_properties(libavutil PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/lib/${ANDROID_ABI}/libavutil.a)
set_target_properties(libswresample PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/lib/${ANDROID_ABI}/libswresample.a)
set_target_properties(libswscale PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/lib/${ANDROID_ABI}/libswscale.a)


add_library(ffmpegjni SHARED
            FFMpegJNI.cpp)

target_include_directories(ffmpegjni PRIVATE
                           ${distribution_DIR}/include
                           )

# Include libraries needed for hello-jni lib
target_link_libraries(ffmpegjni
                      android
                      libavcodec
                      libavdevice
                      libavfilter
                      libavformat
                      libavutil
                      libswresample
                      libswscale
                      log)