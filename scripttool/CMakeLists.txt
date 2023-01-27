
project(scripttool)

add_executable(
	${PROJECT_NAME}
)

target_include_directories(${PROJECT_NAME}
	PRIVATE
		source
	PUBLIC
		include
)

target_sources(${PROJECT_NAME}
	PRIVATE
		source/main.cpp
)

# Clang integration
generate_compile_commands_json(
	TARGETS ${PROJECT_NAME}
)
clang_tidy(
	TARGETS ${PROJECT_NAME}
	VERSION 14.0.0
)
clang_format(
	TARGETS ${PROJECT_NAME}
	DEPENDENCY
	VERSION 14.0.0
)
