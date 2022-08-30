cmake_minimum_required(VERSION 3.12)

project (IQViewer VERSION 1.0)

add_definitions(-DUNICODE)

add_executable(IQViewer WIN32 IQViewer.cpp IQViewer.h)

target_link_libraries(IQViewer d2d1 dwrite windowscodecs dxgi d3d10_1)