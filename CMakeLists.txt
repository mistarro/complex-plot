cmake_minimum_required(VERSION 3.10)

project(ComplexPlot VERSION 0.9 LANGUAGES CXX)

# todo: configure src/version.h properly

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(complex-plot
    src/coloring.cpp
    src/function.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/plotwidget.cpp

    src/coloring.h
    src/function.h
    src/mainwindow.h
    src/plotdata.h
    src/plotwidget.h
    src/version.h
    
    src/mainwindow.ui
    
    res/resources.qrc
)

target_include_directories(complex-plot PRIVATE src)
target_compile_features(complex-plot PRIVATE cxx_std_17)
target_link_libraries(complex-plot PRIVATE Qt5::Widgets)

