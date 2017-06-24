cmake_minimum_required(VERSION 3.6.0)

set(distribution_DIR ${CMAKE_SOURCE_DIR}/../../../distribution)

find_library( # Defines the name of the path variable that stores the
              # location of the NDK library.
              log-lib

              # Specifies the name of the NDK library that
              # CMake needs to locate.
              log )

add_library(avcodec SHARED IMPORTED)
add_library(avfilter SHARED IMPORTED)
add_library(avformat SHARED IMPORTED)
add_library(avutil SHARED IMPORTED)
add_library(swresample SHARED IMPORTED)
add_library(swscale SHARED IMPORTED)

set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/ffmpeg/lib/${ANDROID_ABI}/libavcodec.so)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/ffmpeg/lib/${ANDROID_ABI}/libavfilter.so)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/ffmpeg/lib/${ANDROID_ABI}/libavformat.so)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/ffmpeg/lib/${ANDROID_ABI}/libavutil.so)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/ffmpeg/lib/${ANDROID_ABI}/libswresample.so)
set_target_properties(swscale PROPERTIES IMPORTED_LOCATION
    ${distribution_DIR}/ffmpeg/lib/${ANDROID_ABI}/libswscale.so)


set(SOURCE_FILES encodeVideoOnly.c encodeVideoOnly.h ffmpeg.c)
add_library(ffmpeg_wrap SHARED ${SOURCE_FILES})
target_link_libraries(ffmpeg_wrap avcodec avfilter avformat avutil ${log-lib})

set(SOURCE_FILES_TESTS test/main.c encodeVideoOnly.c encodeVideoOnly.h)
add_executable(ffmpeg_test ${SOURCE_FILES_TESTS})
target_link_libraries(ffmpeg_test avcodec avfilter avformat avutil ${log-lib})

target_include_directories(ffmpeg_wrap PRIVATE
                           ${distribution_DIR}/ffmpeg/include/
                           )

target_include_directories(ffmpeg_test PRIVATE
                           ${distribution_DIR}/ffmpeg/include/
                           )

target_compile_options(ffmpeg_test PRIVATE "-w")
target_compile_options(ffmpeg_wrap PRIVATE "-w")
