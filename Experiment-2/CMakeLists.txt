cmake_minimum_required(VERSION 3.16)

project(LL1_Analyzer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Widgets QUIET)
if(Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
else()
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    set(QT_VERSION_MAJOR 5)
endif()

set(APP_SOURCES
    main.cpp

    ui/mainwindow.cpp
    ui/mainwindow.h
    ui/mainwindow.ui

    src/grammar.cpp
    src/symbols.cpp
    src/utils.cpp
)

set(TEST_SOURCES
    test/test_ll_1_analyzer.cpp
    src/grammar.cpp
    src/symbols.cpp
    src/utils.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(LL1_Analyzer
        MANUAL_FINALIZATION
        ${APP_SOURCES}
    )
    # Define target properties for Android with Qt 6 as:
    #    set_property(TARGET LL1_Analyzer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
    #                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
    # For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(LL1_Analyzer SHARED
            ${APP_SOURCES}
        )
        # Define properties for Android with Qt 5 after find_package() calls as:
        #    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(LL1_Analyzer
            ${APP_SOURCES}
        )
    endif()
endif()

target_link_libraries(LL1_Analyzer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(LL1_Analyzer
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(Qt6_FOUND)
    if(${Qt6_VERSION} VERSION_LESS 6.1.0)
        set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.LL1_Analyzer)
    endif()
endif()
set_target_properties(LL1_Analyzer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# include(GNUInstallDirs)
# install(TARGETS LL1_Analyzer
#     BUNDLE DESTINATION .
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(LL1_Analyzer)
endif()

find_package(GTest REQUIRED)
include(GoogleTest)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_executable("Test-Grammar"
    ${TEST_SOURCES}
)

target_include_directories("Test-Grammar" PRIVATE "include")
target_link_libraries("Test-Grammar" PRIVATE GTest::gtest_main)

gtest_discover_tests("Test-Grammar")
