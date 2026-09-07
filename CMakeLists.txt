cmake_minimum_required(VERSION 3.20)
project(MultithreadedSorting LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "MultithreadedSorting requires Windows and the Windows SDK (DirectX 11).")
endif()
find_package(Threads REQUIRED)

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/MultithreadedSorting/Libs/imgui")
add_library(MultithreadedSortingImGui STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_win32.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_dx11.cpp"
)
target_include_directories(MultithreadedSortingImGui PUBLIC "${IMGUI_DIR}" "${IMGUI_DIR}/backends")
target_compile_features(MultithreadedSortingImGui PRIVATE cxx_std_20)
target_compile_definitions(MultithreadedSortingImGui PRIVATE UNICODE _UNICODE)
target_link_libraries(MultithreadedSortingImGui PUBLIC d3d11 dxgi d3dcompiler dwmapi imm32 user32 gdi32)
set_target_properties(MultithreadedSortingImGui PROPERTIES CXX_EXTENSIONS OFF)

add_executable(MultithreadedSorting
    MultithreadedSorting/main.cpp
    MultithreadedSorting/Data/DataGenerator.cpp
    MultithreadedSorting/Sorting/BubbleSort.cpp
    MultithreadedSorting/Sorting/SelectionSort.cpp
    MultithreadedSorting/Sorting/QuickSort.cpp
    MultithreadedSorting/Sorting/MergeSort.cpp
    MultithreadedSorting/Threading/ThreadManager.cpp
    MultithreadedSorting/Visualization/Visualizer.cpp
)
target_compile_features(MultithreadedSorting PRIVATE cxx_std_20)
set_target_properties(MultithreadedSorting PROPERTIES CXX_EXTENSIONS OFF)
target_compile_definitions(MultithreadedSorting PRIVATE UNICODE _UNICODE)
target_link_libraries(MultithreadedSorting PRIVATE MultithreadedSortingImGui Threads::Threads)
if(MSVC)
    target_compile_options(MultithreadedSorting PRIVATE /W3 /permissive- /utf-8)
    target_compile_options(MultithreadedSortingImGui PRIVATE /utf-8)
endif()
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT MultithreadedSorting)
