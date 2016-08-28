cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(fgcov)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 11)

add_library(libfgcov SHARED
	md5.h
	md5.cpp
	lib.cpp
)

target_link_libraries(libfgcov
	PRIVATE
		dl
)

target_compile_options(libfgcov
	PRIVATE
		-fPIC
)

set_target_properties(libfgcov
	PROPERTIES PREFIX ""
)

# voodo magic to get the relative output directory
# and filename for libfgcov.so
cmake_policy(SET CMP0026 OLD)
get_property(libfgcov_output_file TARGET libfgcov PROPERTY LOCATION)
file(RELATIVE_PATH libfgcov_ouput_file ${CMAKE_CURRENT_BINARY_DIR} ${libfgcov_output_file})
get_filename_component(libfgcov_output_dir ${libfgcov_ouput_file} DIRECTORY)
get_filename_component(libfgcov_output_name ${libfgcov_ouput_file} NAME)

# more voodo magic to compile the contents of libfgcov.so into
# an object file so it can be emdded into the final executable
# the `cd` commands are needed because objcopy takes the name
# of the input file to generate symbols
add_custom_command(OUTPUT libfgcov.o
	COMMAND cd "${libfgcov_output_dir}" && objcopy -B i386:x86-64 -I binary -O elf64-x86-64 "${libfgcov_output_name}" "${CMAKE_CURRENT_BINARY_DIR}/libfgcov.o" && cd -
	DEPENDS libfgcov
)

add_executable(fgcov
	main.cpp
	libfgcov.o
)
