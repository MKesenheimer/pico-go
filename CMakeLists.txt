cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_FETCH_FROM_GIT on)
include(pico_sdk_import.cmake)

project(pico-go
        LANGUAGES ASM C CXX
        VERSION 1.0
        DESCRIPTION "PicoGo"
)

# set(CMAKE_C_STANDARD 11)
# set(CMAKE_CXX_STANDARD 19)

set(FILE_ELF ${CMAKE_PROJECT_NAME}.elf)
set(FILE_HEX ${CMAKE_PROJECT_NAME}.hex)

# Initialize the SDK
pico_sdk_init()
add_executable(${CMAKE_PROJECT_NAME} 
    main.cpp
)

pico_generate_pio_header(${CMAKE_PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/st7789_lcd.pio
)

pico_generate_pio_header(${CMAKE_PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/spi.pio
)


target_sources(${CMAKE_PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/trsensor.cpp
        ${CMAKE_CURRENT_LIST_DIR}/motor.cpp
        ${CMAKE_CURRENT_LIST_DIR}/bluetooth.cpp
        ${CMAKE_CURRENT_LIST_DIR}/gfx.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${CMAKE_PROJECT_NAME}
  pico_stdlib
  hardware_uart
  hardware_pio
  hardware_spi
  hardware_pwm
)

pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
#pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 1)
pico_add_extra_outputs(${CMAKE_PROJECT_NAME})

# add flash target (make flash)
add_custom_target(flash DEPENDS ${PROJECT_NAME}.elf)
add_custom_command(TARGET flash
    USES_TERMINAL
    COMMAND sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program ${PROJECT_NAME}.elf verify reset exit")
add_custom_target(pflash DEPENDS ${PROJECT_NAME}.uf2)
    add_custom_command(TARGET pflash
        USES_TERMINAL
        COMMAND picotool load ${PROJECT_NAME}.uf2 -f && picotool reboot)

# add debug target (make debug)
#set(CMAKE_C_GDB ${GCC_PATH}/arm-none-eabi-gdb CACHE PATH "" FORCE)
add_custom_target(debug DEPENDS ${PROJECT_NAME}.elf)
add_custom_command(TARGET debug
    USES_TERMINAL
    COMMAND xterm -e sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" &)
