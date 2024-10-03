cmake_minimum_required(VERSION ${CMAKE_VERSION})
project(SHessYoachServer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost 1.70 REQUIRED COMPONENTS system thread)

add_executable(${PROJECT_NAME} main.cpp server.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})
