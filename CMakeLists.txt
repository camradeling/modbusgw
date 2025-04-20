cmake_minimum_required(VERSION 3.6)
project(modbusgw)
option(DEBUG_BUILD "Option description" OFF)
option(DOINSTALL "Option description" OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(PROJECT_NAME modbusgw)
message(STATUS "CMAKE_BINARY_DIR = ${CMAKE_BINARY_DIR}")
if(DEBUG_BUILD)
    message(STATUS "DEBUG_BUILD = ON")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fno-omit-frame-pointer -fasynchronous-unwind-tables")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fno-omit-frame-pointer -fasynchronous-unwind-tables")
else()
    message(STATUS "DEBUG_BUILD = OFF")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Os -s")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Os -s")
endif()

message(STATUS "DOINSTALL = ${DOINSTALL}")


message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
include_directories(submodules/channellib)
include_directories(submodules/threadlib)
include_directories(submodules/threadlib/messagequeue)
include_directories(submodules/modbuslib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR})

add_subdirectory(submodules/channellib)
add_subdirectory(submodules/threadlib)

# Исходники для библиотеки
set(LIB_SOURCES
    modbus_gateway.cpp
    modbus_slave_adapter.cpp
    modbus_master_adapter.cpp
    submodules/modbuslib/modbus_client.cpp
    submodules/modbuslib/modbus_crc_tables.c
    utils/timespec.cpp
)

add_custom_target(static_libs)
add_dependencies(static_libs ProgramThread_static ChannelLib_static)

# Создаем статическую библиотеку
add_library(modbusgw_static STATIC ${LIB_SOURCES})
set_target_properties(modbusgw_static PROPERTIES OUTPUT_NAME "modbusgw_static")
target_link_libraries(modbusgw_static
    libChannelLib_static.a
    libProgramThread_static.a
    pthread
    mxml
)

add_dependencies(modbusgw_static static_libs)

# Создаем исполняемый файл
add_executable(modbusgw main.cpp)
target_link_libraries(modbusgw
    libmodbusgw_static.a
    libChannelLib_static.a
    libProgramThread_static.a
    pthread
    mxml
)

add_dependencies(modbusgw modbusgw_static)

install(TARGETS ${PROJECT_NAME}
        DESTINATION /usr/bin)
install(FILES install/config.xml
        DESTINATION /etc/modbusgw)
install(FILES install/modbusgw.service
        DESTINATION /etc/systemd/system)
