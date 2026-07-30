# Install script for directory: C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Zephyr-Kernel")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/arm-toolchain/bin/arm-none-eabi-objdump.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/machine" TYPE FILE FILES
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/ansi.h"
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/_arc4random.h"
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/_default_types.h"
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/endian.h"
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/fastmath.h"
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/fenv-softfloat.h"
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/ieeefp.h"
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/malloc.h"
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/setjmp.h"
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/stdlib.h"
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/termios.h"
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/_time.h"
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/time.h"
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/_types.h"
    "C:/zephyrproject/modules/lib/picolibc/newlib/libc/include/machine/types.h"
    )
endif()

