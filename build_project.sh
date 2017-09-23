#!/bin/zsh
pwd
cd build
cmake -DTOOLCHAIN_DIR=/home/sunqi/program/gcc-arm-none-eabi-5_4-2016q3  \
      -DCMAKE_TOOLCHAIN_FILE=/home/sunqi/Desktop/mcu_proj/conf/toolchain-arm-eabi-gcc.cmake   ..
make