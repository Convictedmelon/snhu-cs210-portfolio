cmake_minimum_required(VERSION 3.20)
project(CornerGrocer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Strong warnings for AppleClang / Clang / GCC
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_executable(CornerGrocer
    src/main.cpp
    src/FrequencyTable.cpp
    src/FrequencyTable.hpp
        src/FrequencyTable.cpp
)

# Make the app run from project root (so relative data/ works)
# (I think thhis is mainly used by Visual Studio; I use Clion and CLion lets us set Working Dir in Run Config.
# Keeping this property here harmless; ill set Working Dir in CLion too.)
set_target_properties(CornerGrocer PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)