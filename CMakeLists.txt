cmake_minimum_required(VERSION 2.8)

project(aducftdi)

set(CMAKE_C_FLAGS "-std=c99")

add_definitions(-D_GNU_SOURCE)

add_executable(aducftdi 
  src/main.c 
  src/aducloader.c 
  src/hex.c 
  src/ftdiserial.c
  src/timeout.c)


target_link_libraries (aducftdi ftdi) 

