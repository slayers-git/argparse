# This file is part of the argparse library.
# Copyright (C) 2021 by Sergey Lafin
# Licensed under the LGPL v2.1, see the file LICENSE in base directory.

cmake_minimum_required (VERSION 3.12)
project (argparse)

set (CMAKE_C_FLAGS "-O3")
option (ARGPARSE_ERRORS "builtin error description via arg_geterror ()" OFF)
option (ARGPARSE_EXAMPLES "build examples" OFF)

set (argparse_h
	${PROJECT_SOURCE_DIR}/argparse.h)
set (argparse_c
	${PROJECT_SOURCE_DIR}/argparse.c)
set (argparse_example_c
	${PROJECT_SOURCE_DIR}/example.c)
add_library (argparse SHARED
	${argparse_h}
	${argparse_c})

set_target_properties (argparse PROPERTIES OUTPUT_NAME "argparse")

if (ARGPARSE_ERRORS)
	target_compile_definitions (argparse PUBLIC ARGPARSE_ERRORS)
endif()

if (ARGPARSE_EXAMPLES)
	add_executable (argparse_ex
		${argparse_example_c})
	target_link_libraries (argparse_ex argparse)
endif()

set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

install (TARGETS argparse
	 LIBRARY DESTINATION lib
	 ARCHIVE DESTINATION lib)

install (FILES ${argparse_h} DESTINATION include)
