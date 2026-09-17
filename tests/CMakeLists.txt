find_package(Python3 REQUIRED COMPONENTS Interpreter)

if(BUILD_SHARED_LIBS)
    set(TEST_LIBRARY munbyn)
else()
    add_library(munbyn_test_driver SHARED ../src/munbyn_printer.c)
    set(TEST_LIBRARY munbyn_test_driver)
endif()

if(UNIX)
    add_test(NAME printer_protocol
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_protocol.py
                $<TARGET_FILE:${TEST_LIBRARY}>)
endif()

add_test(NAME installed_package
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_install.py
            ${CMAKE_COMMAND} ${CMAKE_BINARY_DIR})
