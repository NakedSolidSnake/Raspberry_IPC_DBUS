cmake_minimum_required(VERSION 3.7.0)

project(dbus)

set (CMAKE_C_FLAGS "-D_GNU_SOURCE -Wall -std=c99")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_BUILD_TYPE Debug)


if(DEFINED ARCH)
  if(${ARCH} STREQUAL "RASPBERRY")
    message("-- Created ${ARCH} compilation.")  
    set (LIBS hardware interface)
    set (FOLDER "raspberry")  
  elseif(${ARCH} STREQUAL "PC")
    message("-- Created ${ARCH} compilation.") 
    set (LIBS interface)
    set (FOLDER "pc")
  endif()
else()
  message(WARNING "")
  message("-- You must specify the ARCH variable")
  message("-- cmake -DARCH=PC  ..")
  message("-- or")
  message("-- cmake -DARCH=RASPBERRY  ..")
  message("")
  message(FATAL_ERROR "")
endif()

find_package(PkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(DBUS "dbus-1")
  if (DBUS_FOUND)

    # build lib interface
    add_library(interface "")
    target_link_libraries(interface PUBLIC ${DBUS_LIBRARIES})
    target_sources(interface
      PUBLIC 
      ${CMAKE_CURRENT_LIST_DIR}/src/led_interface.c
      ${CMAKE_CURRENT_LIST_DIR}/src/button_interface.c
      )

    target_include_directories(interface 
      PUBLIC 
      ${CMAKE_CURRENT_LIST_DIR}/include  
      )
    add_definitions(${DBUS_CFLAGS} ${DBUS_CFLAGS_OTHER})
    # install(TARGETS interface DESTINATION bin)

  endif()
endif()

#build launch_processes
add_executable(launch_processes "")

target_sources(launch_processes
  PUBLIC 
  ${CMAKE_CURRENT_LIST_DIR}/src/launch_processes.c
  )

target_include_directories(launch_processes 
  PUBLIC 
  ${CMAKE_CURRENT_LIST_DIR}/lib  
  )

# install(TARGETS launch_processes DESTINATION bin)

# build led_process
add_executable(led_process "")

target_link_libraries(led_process PUBLIC ${LIBS})

target_sources(led_process
  PUBLIC 
  ${CMAKE_CURRENT_LIST_DIR}/src/platform/${FOLDER}/led_process.c
  )

target_include_directories(led_process 
  PUBLIC 
  ${CMAKE_CURRENT_LIST_DIR}/include  
  )

# install(TARGETS led_process DESTINATION bin)

# build button_process
add_executable(button_process "")

target_link_libraries(button_process PUBLIC ${LIBS})

target_sources(button_process
  PUBLIC 
  ${CMAKE_CURRENT_LIST_DIR}/src/platform/${FOLDER}/button_process.c
  )

target_include_directories(button_process 
  PUBLIC 
  ${CMAKE_CURRENT_LIST_DIR}/include  
  )

# install(TARGETS button_process DESTINATION bin)

file(COPY config/kill_process.sh DESTINATION bin)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/config/org.solid.button.conf DESTINATION /etc/dbus-1/system.d)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/config/org.solid.led.conf DESTINATION /etc/dbus-1/system.d)
