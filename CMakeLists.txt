cmake_minimum_required(VERSION ${CMAKE_VERSION})
project(SHessYoachServer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Boost 1.70 REQUIRED COMPONENTS system thread)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPQXX REQUIRED libpqxx)

find_package(PostgreSQL REQUIRED)

include_directories(${LIBPQXX_INCLUDE_DIRS} ${PostgreSQL_INCLUDE_DIRS})
link_directories(${LIBPQXX_LIBRARY_DIRS} ${PostgreSQL_LIBRARY_DIRS})

add_executable(${PROJECT_NAME} main.cpp requestParser.cpp sqlRequests.cpp)

target_include_directories(
  ${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIRS} ${LIBPQXX_INCLUDE_DIRS}
                          ${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(
  ${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES} ${LIBPQXX_LIBRARIES}
                          ${PostgreSQL_LIBRARIES})
