cmake_minimum_required (VERSION 3.10)
project (argparse)

set (CMAKE_C_FLAGS "-O3")

set (argparse_h
	${PROJECT_SOURCE_DIR}/argparse.h)
set (argparse_c
	${PROJECT_SOURCE_DIR}/argparse.c)
add_library (argparse SHARED
	${argparse_c})

set_target_properties (argparse PROPERTIES OUTPUT_NAME "argparse")

set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

install (TARGETS argparse
	 LIBRARY DESTINATION lib
	 ARCHIVE DESTINATION lib)

install (FILES ${argparse_h} DESTINATION include)
