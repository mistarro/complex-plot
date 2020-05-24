cmake_minimum_required(VERSION 3.10)

project(ComplexPlot VERSION 0.9 LANGUAGES CXX)

# todo: configure src/version.h properly

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Threads REQUIRED)

add_executable(complex-plot
    src/main.cpp
    src/engine/coloring.cpp
    src/engine/function.cpp
    src/ui/mainwindow.cpp
    src/ui/plotwidget.cpp

    src/engine/coloring.hpp
    src/engine/engine.hpp
    src/engine/function.hpp
    src/engine/plotdata.hpp
    src/ui/mainwindow.hpp
    src/ui/plotwidget.hpp
    src/version.hpp
    
    src/ui/mainwindow.ui
    
    res/resources.qrc
)

target_include_directories(complex-plot PRIVATE src)
target_compile_features(complex-plot PRIVATE cxx_std_17)
target_link_libraries(complex-plot PRIVATE Qt5::Widgets Threads::Threads)
target_compile_definitions(complex-plot PRIVATE QT_NO_EMIT)
