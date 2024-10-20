cmake_minimum_required(VERSION 3.14)
project(MyProject)

# Ensure C++11 or higher is used
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Use the modern FindPython module
find_package(Python REQUIRED COMPONENTS Interpreter Development)

# Find pybind11
find_package(pybind11 REQUIRED)

# Specify the path to your local CURL installation
set(CURL_INCLUDE_DIR "C:/Users/drkab/OneDrive/Desktop/C++/curl/include")
set(CURL_LIBRARIES "C:/Users/drkab/OneDrive/Desktop/C++/curl/build/lib/libcurl.dll.a")  # Updated to the correct path

# Include CURL directories directly
include_directories(${CURL_INCLUDE_DIR})

# Add the main executable
add_executable(FreeRice src/FreeRice.cpp)

# Link libraries: pybind11 for embedding Python and CURL
target_link_libraries(FreeRice PRIVATE pybind11::embed ${CURL_LIBRARIES} ${Python_LIBRARIES})

# Include directories for Python, CURL, and pybind11
target_include_directories(FreeRice PRIVATE ${Python_INCLUDE_DIRS} ${CURL_INCLUDE_DIR} ${pybind11_INCLUDE_DIRS})

# Include the Python module (synonyms.py) in the build directory
add_custom_target(CopyPythonFiles ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/python ${CMAKE_BINARY_DIR}/python)