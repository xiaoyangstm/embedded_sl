# Install script for directory: C:/Users/33846/.platformio/packages/framework-zephyr

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

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/Users/33846/.platformio/packages/toolchain-gccarmnoneeabi@1.80201.181220/bin/arm-none-eabi-objdump.exe")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/zephyr/arch/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/zephyr/lib/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/zephyr/soc/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/zephyr/boards/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/zephyr/subsys/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/zephyr/drivers/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/_bare_module/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/acpica/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/cmsis/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/cmsis-dsp/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/cmsis-nn/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/cmsis_6/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/fatfs/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/hal_nordic/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/hostap/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/liblc3/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/libmctp/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/libmetal/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/littlefs/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/loramac-node/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/lvgl/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/mbedtls/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/mcuboot/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/mipi-sys-t/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/nrf_wifi/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/open-amp/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/openthread/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/percepio/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/picolibc/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/segger/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/tinycrypt/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/uoscore-uedhoc/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/modules/zcbor/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/zephyr/kernel/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/zephyr/cmake/flash/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/zephyr/cmake/usage/cmake_install.cmake")
  include("H:/nrf54l15/spi/.pio/build/seeed-xiao-nrf54l15/zephyr/cmake/reports/cmake_install.cmake")

endif()

