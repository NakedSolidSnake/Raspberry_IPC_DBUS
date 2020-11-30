cmake_minimum_required(VERSION 3.7.0)

project(dbus)

set (CMAKE_C_FLAGS "-D_GNU_SOURCE -Wall -std=c99")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_BUILD_TYPE Debug)

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(DBUS "dbus-1")
  if (DBUS_FOUND)
    # build led_process
    add_executable(led_process "")
    target_sources(led_process 
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/led_process.c
    )
    target_link_libraries(led_process PUBLIC ${DBUS_LIBRARIES} hardware)
    add_definitions(${DBUS_CFLAGS} ${DBUS_CFLAGS_OTHER})
    install(TARGETS led_process DESTINATION bin)

    # build button_process
    add_executable(button_process "")
    target_sources(button_process 
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/src/button_process.c
    )
    target_link_libraries(button_process PUBLIC ${DBUS_LIBRARIES} hardware)
    add_definitions(${DBUS_CFLAGS} ${DBUS_CFLAGS_OTHER})
    install(TARGETS button_process DESTINATION bin)

  endif()
endif()


#build launch_processes
add_executable(launch_processes "")

target_sources(launch_processes
  PUBLIC 
  ${CMAKE_CURRENT_LIST_DIR}/src/launch_processes.c
  )
 
install(TARGETS launch_processes DESTINATION bin)

file(COPY config/kill_process.sh DESTINATION bin)