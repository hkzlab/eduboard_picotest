# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME} 
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

add_subdirectory(modules/pico-interfaces)
add_subdirectory(modules/pico-mcp23017)

add_dependencies(pico_mcp23017 pico_interfaces)

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    hardware_pio
    pico_mcp23017
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)
