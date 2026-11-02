cmake_minimum_required(VERSION 3.20)

project(VoltGuard
    VERSION 1.0.0
    DESCRIPTION "Physics-Aware ICS/SCADA Intrusion Detection System"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt automatic meta-object processing
set(CMAKE_AUTOMOC ON)

# Find Qt 6 Widgets
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Synthetic traffic generator
add_executable(voltguard_generator
    src/cpp/generator/main.cpp
)

# Modbus/TCP parser
add_executable(voltguard_parser
    src/cpp/parser/parser.cpp
)

# Native Qt 6 dashboard
add_executable(voltguard_dashboard
    src/cpp/dashboard/main.cpp
    src/cpp/dashboard/mainwindow.cpp
    src/cpp/dashboard/mainwindow.h
)

target_link_libraries(voltguard_dashboard
    PRIVATE Qt6::Widgets
)

# Compiler warnings
if(MSVC)
    target_compile_options(voltguard_generator PRIVATE /W4)
    target_compile_options(voltguard_parser PRIVATE /W4)
    target_compile_options(voltguard_dashboard PRIVATE /W4)
else()
    target_compile_options(voltguard_generator PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(voltguard_parser PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(voltguard_dashboard PRIVATE -Wall -Wextra -Wpedantic)
endif()