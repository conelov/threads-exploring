cmake_minimum_required(VERSION 3.16)
project(dataRace)
set(CMAKE_CXX_STANDARD 20)

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME}
  main.cpp
  )
target_link_libraries(${PROJECT_NAME} PRIVATE
  Threads::Threads
  )
target_include_directories(${PROJECT_NAME} PRIVATE
  3rdparty/range_v3/include
  )