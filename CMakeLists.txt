Cmake_minimum_required(VERSION 3.8) # 版本

project("ImageSave") # 项目名称

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)  # ui_xxx.h
set(CMAKE_AUTOMOC ON) # 信号槽的头文件和源文件 moc_XXX.cpp
set(CMAKE_AUTORCC ON) # 资源文件 qrc文件

# 设置Qt5的安装路径
set(Qt5_DIR "D:/Qt/Qt5.14.2/5.14.2/msvc2017_64/lib/cmake/Qt5")

find_package(Qt5 REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCE
	main.cpp
	QtWidgetsApplication2.h
	QtWidgetsApplication2.cpp
	QtWidgetsApplication2.qrc
	QtWidgetsApplication2.ui
	imageprocessor.h
	imageprocessor.cpp
)
if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SOURCE})
else()
    add_executable(${PROJECT_NAME} ${PROJECT_SOURCE})
endif()
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)