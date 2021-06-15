cmake_minimum_required(VERSION 3.16.0)
project(linux-enable-ir-emitter VERSION 0.9.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp 
src/widget/home/Con_Home.hpp src/widget/home/Con_Home.cpp src/widget/home/Ui_Home.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)