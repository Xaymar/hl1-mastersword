# Copyright 2023 Michael Fabian Dirks <info@xamyar.com>
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

project(scripttool)

# Link-Time optimizations
include("CheckIPOSupported")
check_ipo_supported(RESULT D_HAS_IPO)

# Target
add_executable(
	${PROJECT_NAME}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	# C++ 17 with no extensions.
	CXX_STANDARD 17
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	# Don't require memory locations to be fixed.
	POSITION_INDEPENDENT_CODE ON
	# Link-Time optimizations
	INTERPROCEDURAL_OPTIMIZATION ON
	# No prefix for generated binaries.
	PREFIX ""
	IMPORT_PREFIX ""
)

# Content
file(GLOB_RECURSE PROJECT_SOURCE "source/*")
file(GLOB_RECURSE PROJECT_INCLUDE "include/*")
source_group(TREE "${PROJECT_SOURCE_DIR}" FILES ${PROJECT_SOURCE} ${PROJECT_INCLUDE})
target_sources(${PROJECT_NAME}
	PRIVATE
		${PROJECT_SOURCE}
	PUBLIC
		${PROJECT_INCLUDE}
)

target_include_directories(${PROJECT_NAME}
	PRIVATE
		source
	PUBLIC
		include
)

# Clang integration
generate_compile_commands_json(
	TARGETS ${PROJECT_NAME}
)
clang_tidy(
	TARGETS ${PROJECT_NAME}
	GLOBAL
	VERSION 14.0.0
)
clang_format(
	TARGETS ${PROJECT_NAME}
	DEPENDENCY
	GLOBAL
	VERSION 14.0.0
)

# Install
install(
	TARGETS ${PROJECT_NAME}
	RUNTIME LIBRARY
)
if(MSVC)
	install(
		FILES $<TARGET_PDB_FILE:${PROJECT_NAME}>
		TYPE BIN
		OPTIONAL
	)
endif()
