add_library(mqtt_pico mqtt_pico.c)
target_include_directories(mqtt_pico PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(mqtt_pico PUBLIC
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
    pico_mbedtls
    pico_lwip_mbedtls
)

# if credentials.h not exists, copy from template
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/examples/secret/credentials.h)
    message(STATUS "Gerando credentials.h a partir de credentials.h.example")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/secret/credentials.h.example
        ${CMAKE_CURRENT_SOURCE_DIR}/examples/secret/credentials.h
        COPYONLY
    )
endif()

# Compilar exemplos, se existirem
file(GLOB MQTT_PICO_EXAMPLES "examples/*.c")
foreach(EXAMPLE_FILE ${MQTT_PICO_EXAMPLES})
    #extrai o nome do arquivo sem extensão
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_FILE} NAME_WE)

    add_executable(${EXAMPLE_NAME} ${EXAMPLE_FILE})
    pico_set_program_name(${EXAMPLE_NAME} "MQTT PICO Example")
    pico_set_program_version(${EXAMPLE_NAME} "0.1")

    # UART and USB
    pico_enable_stdio_usb(${EXAMPLE_NAME} 1)
    pico_enable_stdio_uart(${EXAMPLE_NAME} 1)

    target_link_libraries(${EXAMPLE_NAME} PRIVATE
        mqtt_pico
    )

    # Include directorie for config files (lwipopts.h, etc.)
    target_include_directories(${EXAMPLE_NAME}  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/examples/secret)

    #gera .uf2, .elf, etc
    pico_add_extra_outputs(${EXAMPLE_NAME})
endforeach()
