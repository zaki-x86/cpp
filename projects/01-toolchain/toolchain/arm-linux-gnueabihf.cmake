# ARM Linux cross-compilation toolchain
# Requires: sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(TOOLCHAIN_PREFIX arm-linux-gnueabihf)

find_program(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}-gcc)
find_program(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
find_program(CMAKE_AR           ${TOOLCHAIN_PREFIX}-ar)
find_program(CMAKE_STRIP        ${TOOLCHAIN_PREFIX}-strip)

set(CMAKE_C_COMPILER_TARGET   ${TOOLCHAIN_PREFIX})
set(CMAKE_CXX_COMPILER_TARGET ${TOOLCHAIN_PREFIX})

set(CMAKE_SYSROOT /usr/${TOOLCHAIN_PREFIX})
set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

add_compile_options(-marm -mfpu=neon -mfloat-abi=hard -march=armv7-a)
