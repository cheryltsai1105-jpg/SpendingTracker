
cmake_minimum_required(VERSION 3.16)
project(SpendingTracker LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

if(MSVC)
    if(DEFINED Qt6_DIR)
        # user provided Qt6_DIR on command line; respect it
    elseif(EXISTS "C:/Qt/6.11.1/msvc_64/lib/cmake/Qt6")
        set(Qt6_DIR "C:/Qt/6.11.1/msvc_64/lib/cmake/Qt6" CACHE PATH "Qt6 directory" FORCE)
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.11.1/msvc_64")
    elseif(EXISTS "C:/Qt/6.11.1/msvc2022_64/lib/cmake/Qt6")
        set(Qt6_DIR "C:/Qt/6.11.1/msvc2022_64/lib/cmake/Qt6" CACHE PATH "Qt6 directory" FORCE)
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.11.1/msvc2022_64")
    elseif(EXISTS "C:/Qt/6.11.1/mingw_64/lib/cmake/Qt6")
        message(FATAL_ERROR "MSVC build detected, but only Qt6 for MinGW is installed at C:/Qt/6.11.1/mingw_64. Install Qt6 for MSVC (msvc_64) or reconfigure with a MinGW generator.")
    else()
        message(FATAL_ERROR "MSVC build detected, but no Qt6 installation was found under C:/Qt/6.11.1. Install Qt6 for MSVC or set Qt6_DIR to a valid MSVC Qt install.")
    endif()
else()
    if(EXISTS "C:/Qt/6.11.1/mingw_64/lib/cmake/Qt6")
        set(Qt6_DIR "C:/Qt/6.11.1/mingw_64/lib/cmake/Qt6" CACHE PATH "Qt6 directory")
        list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.11.1/mingw_64")
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(SpendingTracker
    main.cpp
    MainWindow.cpp
    TrackerTab.cpp
    PieChartWidget.cpp
    BarChartWidget.cpp
    GoalsTab.cpp
    HistoryTab.cpp
)

if(MSVC)
    target_compile_options(SpendingTracker PRIVATE "/Zc:__cplusplus")
endif()

target_link_libraries(SpendingTracker
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

# ── Qt5 fallback ──────────────────────────────────────────────────────────────
# find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
# target_link_libraries(SpendingTracker PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)
