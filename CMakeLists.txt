cmake_minimum_required(VERSION 3.20)

project(VoltGuard
    VERSION 1.0.0
    DESCRIPTION "Physics-Aware ICS/SCADA Intrusion Detection System"
    LANGUAGES CXX
)

# --------------------------------------------------
# C++ Configuration
# --------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------
# Qt 6 Configuration
# --------------------------------------------------

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

# --------------------------------------------------
# Target 1: Mock Modbus/TCP Traffic Generator
# --------------------------------------------------

add_executable(voltguard_generator
    src/cpp/generator/main.cpp
)

# --------------------------------------------------
# Target 2: Modbus/TCP Protocol Parser
# --------------------------------------------------

add_executable(voltguard_parser
    src/cpp/parser/parser.cpp
)

# --------------------------------------------------
# Target 3: VoltGuard Desktop Dashboard
# --------------------------------------------------

add_executable(voltguard_dashboard
    src/cpp/dashboard/main.cpp
    src/cpp/dashboard/mainwindow.cpp
    src/cpp/dashboard/mainwindow.h
)

target_link_libraries(voltguard_dashboard
    PRIVATE Qt6::Widgets
)

# --------------------------------------------------
# Compiler Warnings
# --------------------------------------------------

if(MSVC)

    target_compile_options(voltguard_generator PRIVATE /W4)
    target_compile_options(voltguard_parser PRIVATE /W4)
    target_compile_options(voltguard_dashboard PRIVATE /W4)

else()

    target_compile_options(voltguard_generator PRIVATE
        -Wall -Wextra -Wpedantic
    )

    target_compile_options(voltguard_parser PRIVATE
        -Wall -Wextra -Wpedantic
    )

    target_compile_options(voltguard_dashboard PRIVATE
        -Wall -Wextra -Wpedantic
    )

endif()