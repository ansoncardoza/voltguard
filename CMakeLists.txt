
cmake_minimum_required(VERSION 3.20)

project(VoltGuard
    VERSION 1.0.0
    DESCRIPTION "Physics-Aware ICS/SCADA Intrusion Detection System"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(voltguard_generator
    src/cpp/generator/main.cpp
)

add_executable(voltguard_parser
    src/cpp/parser/parser.cpp
)

if(MSVC)
    target_compile_options(voltguard_generator PRIVATE /W4)
    target_compile_options(voltguard_parser PRIVATE /W4)
else()
    target_compile_options(voltguard_generator PRIVATE
        -Wall -Wextra -Wpedantic
    )

    target_compile_options(voltguard_parser PRIVATE
        -Wall -Wextra -Wpedantic
    )
endif()
