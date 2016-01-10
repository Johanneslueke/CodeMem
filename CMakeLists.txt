cmake_minimum_required (VERSION 2.8)
project(PlatformAPI)
project(CodeMem)

### SO Library
add_subdirectory(PlatformAPI)

set(SO_SOURCE PlatformAPI/PlatformAPI.cpp)
set(SO_HEADER "include/Platform.h")
add_library(PlatformAPI SHARED ${SO_SOURCE} ${SO_HEADER})


## Excutable
set (CodeMem_VERSION_MAJOR 1)
set (CodeMem_VERSION_MINOR 0)

include_directories(include)
file(GLOB SOURCES "Application.cpp")
file(GLOB SOURCES "source/*.cpp")
add_executable(CodeMem ${SOURCES} Application.cpp)

## Compiler flags
target_link_libraries(CodeMem dl)
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "--std=c++11 -Wall -ldl")       
    set(CMAKE_EXE_LINKER_FLAGS "-ldl") 
endif()



