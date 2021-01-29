cmake_minimum_required(VERSION 2.8)
enable_language(C ASM CXX)
project(thready C ASM)

set(CFLAGS_WARNING "-Wall" "-Wextra" "-Wno-unused-parameter" "-Wno-sign-compare" "-Wno-unused-function" ${EXTRA_WARNINGS} CACHE STRING "Warning flags")
add_compile_options(${CFLAGS_WARNING})

add_library(thready SHARED libthready.c)
target_link_libraries(thready dl)
set_property(TARGET thready PROPERTY VERSION 1.0.0)
configure_file(threadyrun.in threadyrun @ONLY)

install(TARGETS thready
	LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib64")
install(PROGRAMS
	"${CMAKE_CURRENT_BINARY_DIR}/threadyrun"
	DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
