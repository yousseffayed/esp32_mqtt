# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/youss/esp/v5.3.1/esp-idf/components/bootloader/subproject"
  "C:/Users/youss/ssl/bootloader"
  "C:/Users/youss/ssl/bootloader-prefix"
  "C:/Users/youss/ssl/bootloader-prefix/tmp"
  "C:/Users/youss/ssl/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/youss/ssl/bootloader-prefix/src"
  "C:/Users/youss/ssl/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/youss/ssl/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/youss/ssl/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
