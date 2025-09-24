add_library(aht20 aht20.c)
target_include_directories(aht20 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(aht20 PUBLIC
    pico_stdlib
    hardware_i2c
)

# Compilar exemplos, se existirem
file(GLOB AHT20_EXAMPLES "examples/*.c")
foreach(EXAMPLE_FILE ${AHT20_EXAMPLES})
    # extrai o nome do arquivo sem extensão
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)
    
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_FILE})
    pico_set_program_name(${EXAMPLE_NAME} "AHT20 Example")
    pico_set_program_version(${EXAMPLE_NAME} "0.1")

    # UART and USB
    pico_enable_stdio_usb(${EXAMPLE_NAME} 1)
    pico_enable_stdio_uart(${EXAMPLE_NAME} 1)
    
    # Linka a lib principal + SDK
    target_link_libraries(${EXAMPLE_NAME} PRIVATE aht20)
    
    # gera .uf2, .elf, etc
    pico_add_extra_outputs(${EXAMPLE_NAME})
endforeach()