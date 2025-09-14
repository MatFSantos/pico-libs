add_library(ssd1306 ssd1306.c)
target_include_directories(ssd1306 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(ssd1306 PUBLIC
    pico_stdlib
    hardware_i2c
)

# Compilar exemplos, se existirem
file(GLOB AHT20_EXAMPLES "examples/*.c")
foreach(EXAMPLE_FILE ${AHT20_EXAMPLES})
    # extrai o nome do arquivo sem extensão
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)
    
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_FILE})
    pico_set_program_name(${EXAMPLE_NAME} "SSD1306 Example")
    pico_set_program_version(${EXAMPLE_NAME} "0.1")

    # UART and USB
    pico_enable_stdio_usb(${EXAMPLE_NAME} 1)
    pico_enable_stdio_uart(${EXAMPLE_NAME} 1)
    
    # Linka a lib principal + SDK
    target_link_libraries(${EXAMPLE_NAME} PRIVATE ssd1306 pico_stdlib hardware_i2c)
    
    #gera .uf2, .elf, etc
    pico_add_extra_outputs(${EXAMPLE_NAME})
endforeach()