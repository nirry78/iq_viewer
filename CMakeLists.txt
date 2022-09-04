cmake_minimum_required(VERSION 3.12)

project (IQViewer VERSION 1.0)

add_definitions(-DUNICODE -D_USE_MATH_DEFINES)

add_executable(IQViewer WIN32 IQViewer.cpp IQViewer.h IQData.cpp IQData.h)

target_link_libraries(IQViewer d2d1 dwrite windowscodecs dxgi d3d10_1)

add_executable(IQDataTester IQDataTester.cpp IQDataTester.h IQData.cpp IQData.h)
