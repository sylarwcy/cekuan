cmake_minimum_required(VERSION 3.29)
set(project_name multiStationOCR)
project(${project_name} VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
include(GNUInstallDirs)
set(CMAKE_AUTOUIC ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../bin)

set(QT_VERSION_MAJOR 5)
set(CMAKE_PREFIX_PATH "C:/Qt/5.15.2/msvc2019_64")
set(PYLON_PATH "C:/Program Files/Basler/pylon/Runtime/x64")

add_compile_definitions(_HAS_STD_BYTE=0)
add_compile_definitions(
        NOMINMAX           # 防止 min/max 宏冲突
        WIN32_LEAN_AND_MEAN # 减少 Windows 头文件包含
        _CRT_SECURE_NO_WARNINGS # 禁用某些安全警告
)

find_package(Qt5 COMPONENTS
        Core Gui Network OpenGL PrintSupport Sql WinExtras Widgets Script
        REQUIRED
)

qt5_add_resources(QRC_SOURCES
        "${CMAKE_SOURCE_DIR}/../core_qui/resource/icons.qrc"
        "${CMAKE_SOURCE_DIR}/../core_qui/resource/qm.qrc"
        "${CMAKE_SOURCE_DIR}/../core_qui/resource/qss.qrc"
        "${CMAKE_SOURCE_DIR}/../core_qui/resource/ttf.qrc"
)

file(GLOB SRC_FILES CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/../inc/opencv/include/opencv2/opencv.hpp
        ${CMAKE_SOURCE_DIR}/main/*.h ${CMAKE_SOURCE_DIR}/main/*.cpp  # main
        ${CMAKE_SOURCE_DIR}/app/*.h ${CMAKE_SOURCE_DIR}/app/*.cpp# app
        ${CMAKE_SOURCE_DIR}/form/*.h ${CMAKE_SOURCE_DIR}/form/*.cpp ${CMAKE_SOURCE_DIR}/form/*.ui  # form
        ${CMAKE_SOURCE_DIR}/usercontrol/*.h ${CMAKE_SOURCE_DIR}/usercontrol/*.cpp  # usercontrol
        ${CMAKE_SOURCE_DIR}/../core_qui/*.h ${CMAKE_SOURCE_DIR}/../core_qui/*.cpp  # core_qui
        ${CMAKE_SOURCE_DIR}/../inc/core_db/*.h ${CMAKE_SOURCE_DIR}/../inc/core_db/*.cpp   # core_db
        ${CMAKE_SOURCE_DIR}/../inc/customgraphics/*.h ${CMAKE_SOURCE_DIR}/../inc/customgraphics/*.cpp  # customgraphics
        ${CMAKE_SOURCE_DIR}/../inc/customplot/*.h ${CMAKE_SOURCE_DIR}/../inc/customplot/*.cpp  # customplot
        ${CMAKE_SOURCE_DIR}/../inc/global_data/*.h ${CMAKE_SOURCE_DIR}/../inc/global_data/*.cpp  # global_data
        ${CMAKE_SOURCE_DIR}/../inc/halcon/include/HDevThread.h ${CMAKE_SOURCE_DIR}/../inc/halcon/include/HalconCpp.h # halcon
        ${CMAKE_SOURCE_DIR}/../inc/ini/*.h ${CMAKE_SOURCE_DIR}/../inc/ini/*.cpp  # ini
        ${CMAKE_SOURCE_DIR}/../inc/mulThread/*.h ${CMAKE_SOURCE_DIR}/../inc/mulThread/*.cpp  # mulThread
        ${CMAKE_SOURCE_DIR}/../inc/MVS/includes/*.h ${CMAKE_SOURCE_DIR}/../inc/MVS/includes/*.cpp # MVS
        ${CMAKE_SOURCE_DIR}/../inc/OpenCV/include/opencv2/opencv.hpp  # OpenCV
        ${CMAKE_SOURCE_DIR}/../inc/oracleDB/*.h ${CMAKE_SOURCE_DIR}/../inc/oracleDB/*.cpp # oracleDB
        ${CMAKE_SOURCE_DIR}/../inc/QsLog/*.h ${CMAKE_SOURCE_DIR}/../inc/QsLog/*.cpp  # QsLog
        ${CMAKE_SOURCE_DIR}/../inc/qtpropertybrowser/*.h ${CMAKE_SOURCE_DIR}/../inc/qtpropertybrowser/*.cpp  # qtpropertybrowser
        ${CMAKE_SOURCE_DIR}/../inc/shareMemory/*.h ${CMAKE_SOURCE_DIR}/../inc/shareMemory/*.cpp  # shareMemory
        ${CMAKE_SOURCE_DIR}/imgAI/*.h
        ${CMAKE_SOURCE_DIR}/imgAI/Det/*.h ${CMAKE_SOURCE_DIR}/imgAI/Det/*.cpp
        ${CMAKE_SOURCE_DIR}/imgAI/OCR/*.h ${CMAKE_SOURCE_DIR}/imgAI/OCR/*.cpp
        ${CMAKE_SOURCE_DIR}/imgAI/Preprocess/*.h ${CMAKE_SOURCE_DIR}/imgAI/Preprocess/*.cpp
        ${CMAKE_SOURCE_DIR}/imgAI/Tool/*.h ${CMAKE_SOURCE_DIR}/imgAI/Tool/*.cpp
        ${CMAKE_SOURCE_DIR}/imgAI/ModelManager/*.h ${CMAKE_SOURCE_DIR}/imgAI/ModelManager/*.cpp
)

file(GLOB EXCLUDE_FILES
        ${CMAKE_SOURCE_DIR}/form/frmshowcurve.*
        ${CMAKE_SOURCE_DIR}/form/frmviewdevice.*
        ${CMAKE_SOURCE_DIR}/form/frmview1-.ui
        ${CMAKE_SOURCE_DIR}/../inc/core_qui/dbcountthread.*
        ${CMAKE_SOURCE_DIR}/../inc/mulThread/performancetimer.*
        ${CMAKE_SOURCE_DIR}/../inc/mulThread/ppcimgprocess.*
        ${CMAKE_SOURCE_DIR}/../inc/QsLog/ZLog.h
        ${CMAKE_SOURCE_DIR}/../inc/shareMemory/ShareMem.*

        ${CMAKE_SOURCE_DIR}/../inc/MVS/includes/CameraParams.h
        ${CMAKE_SOURCE_DIR}/../inc/MVS/includes/MvErrorDefine.h
        ${CMAKE_SOURCE_DIR}/../inc/MVS/includes/MvISPErrorDefine.h
        ${CMAKE_SOURCE_DIR}/../inc/MVS/includes/MvObsoleteInterfaces.h
        ${CMAKE_SOURCE_DIR}/../inc/MVS/includes/MvSdkExport.h
        ${CMAKE_SOURCE_DIR}/../inc/MVS/includes/myAngleProcess.*
        ${CMAKE_SOURCE_DIR}/../inc/MVS/includes/ObsoleteCamParams.h
        ${CMAKE_SOURCE_DIR}/../inc/MVS/includes/PixelType.h

)
list(REMOVE_ITEM SRC_FILES ${EXCLUDE_FILES})

add_executable(${project_name} WIN32 MACOSX_BUNDLE
        ${SRC_FILES}
        ${QRC_SOURCES}
)

# 启用预编译头文件支持
target_precompile_headers(${project_name} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/stable.h"
)

target_include_directories(${project_name} PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/../core_qui
        ${CMAKE_SOURCE_DIR}/../inc
        ${CMAKE_SOURCE_DIR}/../inc/MQTT/include
        ${CMAKE_SOURCE_DIR}/../inc/MVS/includes
        ${CMAKE_SOURCE_DIR}/../inc/OpenCV/include
        ${CMAKE_SOURCE_DIR}/../inc/OpenCV/include/opencv2
        ${CMAKE_SOURCE_DIR}/../inc/QsLog ../inc/QsLog
        ${CMAKE_SOURCE_DIR}/../inc/core_db
        ${CMAKE_SOURCE_DIR}/../inc/customgraphics
        ${CMAKE_SOURCE_DIR}/../inc/customplot
        ${CMAKE_SOURCE_DIR}/../inc/global_data
        ${CMAKE_SOURCE_DIR}/../inc/halcon/include
        ${CMAKE_SOURCE_DIR}/../inc/halcon/include/halconcpp
#        ${CMAKE_SOURCE_DIR}/../inc/ini
        ${CMAKE_SOURCE_DIR}/../inc/mulThread
        ${CMAKE_SOURCE_DIR}/../inc/oracleDB
        ${CMAKE_SOURCE_DIR}/../inc/qtpropertybrowser
        ${CMAKE_SOURCE_DIR}/../inc/shareMemory
        ${CMAKE_SOURCE_DIR}/app
        ${CMAKE_SOURCE_DIR}/form
        ${CMAKE_SOURCE_DIR}/usercontrol
        ${CMAKE_SOURCE_DIR}/main
        ${CMAKE_SOURCE_DIR}/imgAI
        ${CMAKE_SOURCE_DIR}/imgAI/Tool
        ${CMAKE_SOURCE_DIR}/imgAI/ModelManager
)

target_compile_definitions(${project_name} PRIVATE
        no_style
)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(MQTT_LIB ${CMAKE_SOURCE_DIR}/../inc/MQTT/lib/Qt5Qmqttd.lib)
    set(OPENCV_LIB ${CMAKE_SOURCE_DIR}/../inc/OpenCV/lib/opencv_world430d.lib)
else ()
    set(MQTT_LIB ${CMAKE_SOURCE_DIR}/../inc/MQTT/lib/Qt5Qmqtt.lib)
    set(OPENCV_LIB ${CMAKE_SOURCE_DIR}/../inc/OpenCV/lib/opencv_world430.lib)
endif ()

target_link_libraries(${project_name} PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Network
        Qt::OpenGL
        Qt::PrintSupport
        Qt::Sql
        Qt::WinExtras
        Qt::Widgets
        gdi32

        # ThirdParty
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/halcon/lib/halcon.lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/halcon/lib/halconcpp.lib
        ${CMAKE_CURRENT_SOURCE_DIR}/../inc/MVS/Libraries/MvCameraControl.lib
        ${OPENCV_LIB}
        ${MQTT_LIB}
)

set(DLL_INI_FILES
        ${CMAKE_SOURCE_DIR}/../inc/MQTT/lib/Qt5Qmqtt.dll
        ${CMAKE_SOURCE_DIR}/../inc/MQTT/lib/Qt5Qmqttd.dll
        ${CMAKE_SOURCE_DIR}/../inc/OpenCV/lib/opencv_world430.dll
        ${CMAKE_SOURCE_DIR}/../inc/OpenCV/lib/opencv_world430d.dll
        ${CMAKE_SOURCE_DIR}/Calibrate.ini
#        ${CMAKE_SOURCE_DIR}/setting.ini
)
foreach(DLL_INI_FILES ${DLL_INI_FILES})
    add_custom_command(
            TARGET ${project_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${DLL_INI_FILES}
            $<TARGET_FILE_DIR:${project_name}>
            COMMENT "Copying ${DLL_INI_FILES} to output directory"
    )
endforeach()

#install(TARGETS ${project_name}
#        BUNDLE DESTINATION .
#        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#)

# Consider using qt_generate_deploy_app_script() for app deployment if
# the project can use Qt 6.3. In that case rerun qmake2cmake with
# --min-qt-version=6.3.
