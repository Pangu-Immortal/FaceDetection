# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.22.1)

# Declares and names the project.

project("faceidentification")
set(pathToOpenCv D:\\opencv\\opencv-4.6.0-android-sdk\\OpenCV-android-sdk\\)
set(libs "${CMAKE_SOURCE_DIR}/jniLibs")
set(PATH_TO_PRE_BUILT ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI})

include_directories(${pathToOpenCv}/sdk/native/jni/include)
include_directories(${CMAKE_SOURCE_DIR}/include/)

add_definitions(-DDLIB_PNG_SUPPORT)
#include_directories(${CMAKE_SOURCE_DIR}/include)
#include_directories(${CMAKE_SOURCE_DIR}/include/seeta)
#include_directories(${CMAKE_SOURCE_DIR}/include/seetaNet)
## Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        faceidentification

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp FaceProcessEngine.cpp FaceProcessEngine.h)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.
add_library(lib_opencv STATIC IMPORTED )
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/jniLibs/${ANDROID_ABI}/libopencv_java4.so )
#add_library(libseetanet2 STATIC IMPORTED )
#set_target_properties(libseetanet2 PROPERTIES
#        IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libseetanet2.so")
#
#add_library(libfd STATIC IMPORTED )
#set_target_properties(libfd PROPERTIES
#        IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libSeetaFaceDetector2.so")
#
#add_library(libfl STATIC IMPORTED )
#set_target_properties(libfl PROPERTIES
#        IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libSeetaFaceLandmarker2.so")
#
#add_library(libfr STATIC IMPORTED )
#set_target_properties(libfr PROPERTIES
#        IMPORTED_LOCATION "${libs}/${ANDROID_ABI}/libSeetaFaceRecognizer2.so")
#set(OpenCV_DIR D:\\opencv\\opencv-4.6.0-android-sdk\\OpenCV-android-sdk\\sdk\native\\jni)
# find_package(OpenCV REQUIRED)

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        faceidentification

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib}
        lib_opencv
        android
        ${PATH_TO_PRE_BUILT}/libdlib.so
        )