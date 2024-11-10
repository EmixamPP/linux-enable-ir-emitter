cmake_minimum_required(VERSION 3.10)
project("linux-enable-ir-emitter dependencies")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/install_dir" CACHE PATH "Default install prefix" FORCE)
endif()

set(OPENCV_GIT_TAG "4.10.0" CACHE STRING "Tag or branch used to checkout when cloning opencv")
set(YAML_GIT_TAG "0.8.0" CACHE STRING "Tag or branch used to checkout when cloning yaml-cpp")
set(ARGPARSE_GIT_TAG "v3.1" CACHE STRING "Tag or branch used to checkout when cloning argparse")
set(SPDLOG_GIT_TAG "v1.15.0" CACHE STRING "Tag or branch used to checkout when cloning argparse")

include(ExternalProject)

ExternalProject_Add(opencv
    GIT_REPOSITORY https://github.com/opencv/opencv.git
    GIT_TAG ${OPENCV_GIT_TAG}
    PREFIX ${CMAKE_BINARY_DIR}/opencv
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS
        -DCMAKE_CXX_STANDARD=20
        -DLIBRARY_OUTPUT_DIRECTORY=lib
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DBUILD_SHARED_LIBS=OFF
        -DOPENCV_GENERATE_PKGCONFIG=YES
        -DBUILD_LIST=videoio,highgui
        -DWITH_V4L=ON
        -DWITH_GTK=ON
        -DZLIB_FOUND=OFF
        -DBUILD_ZLIB=ON
        -DWITH_JPEG=OFF
        -DWITH_PNG=OFF
        -DWITH_TIFF=OFF
        -DWITH_WEBP=OFF
        -DWITH_OPENJPEG=OFF
        -DWITH_JASPER=OFF
        -DWITH_OPENEXR=OFF
        -DWITH_IMGCODEC_HDR=OFF
        -DWITH_IMGCODEC_SUNRASTER=OFF
        -DWITH_IMGCODEC_PXM=OFF
        -DWITH_IMGCODEC_PFM=OFF
        -DWITH_FFMPEG=FALSE
        -DWITH_GSTREAMER=OFF
        -DWITH_1394=OFF
        -DDVIDEOIO_ENABLE_PLUGINS=OFF
        -DWITH_ANDROID_MEDIANDK=OFF
        -DHIGHGUI_ENABLE_PLUGINS=OFF
        -DWITH_VTK=OFF
        -DWITH_PROTOBUF=OFF
        -DOPENCV_DNN_OPENCL=OFF
        -DWITH_VA_INTEL=OFF
        -DWITH_OPENCL=OFF
        -DWITH_OPENCL_SVM=OFF
        -DWITH_OPENCLAMDFFT=OFF
        -DWITH_OPENCLAMDBLAS=OFF
        -DWITH_OPENCL_D3D11_NV=OFF
        -DWITH_CAROTENE=OFF
        -DWITH_CPUFEATURES=OFF
        -DWITH_EIGEN=OFF
        -DWITH_OPENVX=OFF
        -DWITH_DIRECTX=OFF
        -DWITH_VA=OFF
        -DWITH_LAPACK=OFF
        -DWITH_QUIRC=OFF
        -DBUILD_ITT=OFF
        -DWITH_IPP=OFF
        -DBUILD_IPP_IW=OFF
)

ExternalProject_Add(yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG ${YAML_GIT_TAG}
    PREFIX ${CMAKE_BINARY_DIR}/yaml-cpp
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS
        -DCMAKE_CXX_STANDARD=20
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DYAML_BUILD_SHARED_LIBS=OFF
        -DYAML_CPP_BUILD_CONTRIB=OFF
        -DYAML_CPP_BUILD_TOOLS=OFF
)

ExternalProject_Add(argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG ${ARGPARSE_GIT_TAG}
    PREFIX ${CMAKE_BINARY_DIR}/argparse
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS
        -DCMAKE_CXX_STANDARD=20
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DARGPARSE_BUILD_TESTS=OFF
)

ExternalProject_Add(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG ${SPDLOG_GIT_TAG}
    PREFIX ${CMAKE_BINARY_DIR}/spdlog
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS
        -DCMAKE_CXX_STANDARD=20
        -DSPDLOG_USE_STD_FORMAT=ON
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
