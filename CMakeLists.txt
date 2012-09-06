project(aducftdi)

set(CMAKE_C_FLAGS "-std=c99")

add_executable(aducftdi 
  src/main.c 
  src/aducloader.c 
  src/hex.c 
  src/ftdiserial.c
  src/timeout.c)


target_link_libraries (aducftdi ftdi) 