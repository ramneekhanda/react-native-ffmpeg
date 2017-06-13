cmake_minimum_required(VERSION 3.6.0)

set(distribution_DIR ${CMAKE_SOURCE_DIR}/../../../distribution)

add_library(avcodec SHARED IMPORTED)
add_library(avfilter SHARED IMPORTED)
add_library(avformat SHARED IMPORTED)
add_library(avutil SHARED IMPORTED)
add_library(swresample SHARED IMPORTED)
add_library(swscale SHARED IMPORTED)

set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/libs/${ANDROID_ABI}/lib/libavcodec.so)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/libs/${ANDROID_ABI}/lib/libavfilter.so)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/libs/${ANDROID_ABI}/lib/libavformat.so)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/libs/${ANDROID_ABI}/lib/libavutil.so)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/libs/${ANDROID_ABI}/lib/libswresample.so)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/libs/${ANDROID_ABI}/lib/libswscale.so)


set(SOURCE_FILES make_gif.c make_gif.h)
add_library(ffmpeg_wrap SHARED ${SOURCE_FILES})
target_link_libraries(ffmpeg_wrap avcodec avfilter avformat avutil)

set(SOURCE_FILES_TESTS test/main.c make_gif.c make_gif.h)
add_executable(ffmpeg_test ${SOURCE_FILES_TESTS})
target_link_libraries(ffmpeg_test avcodec avfilter avformat avutil)

target_include_directories(ffmpeg_wrap PRIVATE
                           ${distribution_DIR}/libs/${ANDROID_ABI}/include/
                           )

target_include_directories(ffmpeg_test PRIVATE
                           ${distribution_DIR}/libs/${ANDROID_ABI}/include/
                           )
