cmake_minimum_required(VERSION 3.7)

project(mcu)

include(CMakeForceCompiler)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m3)  #设置target处理器类型

find_program(ARM_CC arm-none-eabi-gcc ${TOOLCHAIN_DIR}/bin)
find_program(ARM_CXX arm-none-eabi-g++ ${TOOLCHAIN_DIR}/bin)
find_program(ARM_OBJCOPY arm-none-eabi-objcopy ${TOOLCHAIN_DIR}/bin)
find_program(ARM_SIZE_TOOL arm-none-eabi-size ${TOOLCHAIN_DIR}/bin)

CMAKE_FORCE_C_COMPILER(${ARM_CC} GNU)
CMAKE_FORCE_CXX_COMPILER(${ARM_CXX} GNU)

set(CMAKE_ARM_FLAGS
        "-mcpu=cortex-m3 -mthumb -fno-common -ffunction-sections -fdata-sections"
        )

if (CMAKE_SYSTEM_PROCESSOR STREQUAL "cortex-m3")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_ARM_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_ARM_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CMAKE_ARM_FLAGS}")
else ()
    message(WARNING
            "Processor not recognised in toolchain file, "
            "compiler flags not configured."
            )
endif ()

# fix long strings (CMake appends semicolons)
string(REGEX REPLACE ";" " " CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "")

set(BUILD_SHARED_LIBS OFF)






enable_language(ASM)

#这个地方可以灵活修改

set(BUILD_MODE DEBUG)

set(USER_C_FLAGS "-mcpu=cortex-m3 -mthumb -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding \
        -fno-move-loop-invariants -Wall")
set(USER_CXX_FLAGS  "-std=c++11 -fabi-version=0 -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics")
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
        COMMAND ${ARM_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${ARM_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        )