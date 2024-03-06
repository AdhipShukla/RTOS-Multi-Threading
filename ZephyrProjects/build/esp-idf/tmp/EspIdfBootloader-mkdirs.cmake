# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/adhipshukla/zephyrproject/modules/hal/espressif/components/bootloader/subproject"
  "/home/adhipshukla/zephyrproject/zephyr_codes/projects/build/esp-idf/build/bootloader"
  "/home/adhipshukla/zephyrproject/zephyr_codes/projects/build/esp-idf"
  "/home/adhipshukla/zephyrproject/zephyr_codes/projects/build/esp-idf/tmp"
  "/home/adhipshukla/zephyrproject/zephyr_codes/projects/build/esp-idf/src/EspIdfBootloader-stamp"
  "/home/adhipshukla/zephyrproject/zephyr_codes/projects/build/esp-idf/src"
  "/home/adhipshukla/zephyrproject/zephyr_codes/projects/build/esp-idf/src/EspIdfBootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/adhipshukla/zephyrproject/zephyr_codes/projects/build/esp-idf/src/EspIdfBootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/adhipshukla/zephyrproject/zephyr_codes/projects/build/esp-idf/src/EspIdfBootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
