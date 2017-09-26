cmake_minimum_required(VERSION 3.7)


project(mcu)

enable_language(ASM)

#这个地方可以灵活修改

set(BUILD_MODE DEBUG)

set(USER_C_FLAGS "-mcpu=cortex-m3 -mthumb -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding \
        -fno-move-loop-invariants -Wall")
set(USER_CXX_FLAGS  "-std=c++11 -fabi-version=0 -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics -fpermissive")
#-DUSE_FULL_ASSERT \
set(USER_C_DEFINES
        "-DARM_MATH_CM3 \
    -DSTM32F10X_HD\
    -DTRACE \
    -DOS_USE_TRACE_SEMIHOSTING_STDOUT \
    -DSTM32F10X_HD \
    -DUSE_STDPERIPH_DRIVER \
    -DHSE_VALUE=8000000"
        )

if(BUILD_MODE STREQUAL "DEBUG")
    set(USER_C_FLAGS "${USER_C_FLAGS} -Og -g3")
    set(USER_C_DEFINES "${USER_C_DEFINES} -DDEBUG")
else()
    set(USER_C_FLAGS "${USER_C_FLAGS} -O3")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USER_C_FLAGS} ${USER_C_DEFINES}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USER_C_FLAGS} ${USER_CXX_FLAGS} ${USER_C_DEFINES}")


include_directories(
        ${TOOLCHAIN_DIR}/arm-none-eabi/include
        system/include
        stdlib/Inc
        user/
)

file(GLOB_RECURSE SOURCE_FILES
        #指定启动文件
        system/startup/startup_stm32f103xe.s
        main.c
        user/*.c
        stdlib/Src/*.c
        )

SET(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/system/startup/STM32F103XE_FLASH.ld)

set(CMAKE_EXE_LINKER_FLAGS
        "${CMAKE_EXE_LINKER_FLAGS} -T ${LINKER_SCRIPT} -fmessage-length=0 -fsigned-char -ffreestanding \
          -specs=nano.specs  -specs=nosys.specs -nostartfiles  -Xlinker --gc-sections -Wl,-Map=${PROJECT_NAME}.map")

add_executable(${PROJECT_NAME}.elf ${SOURCE_FILES})

set(HEX_FILE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin)
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        )