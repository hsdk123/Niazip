add_executable(NiazipTests)

enable_testing()

target_link_libraries(NiazipTests PUBLIC
	#gtest_main
	gtest
	gmock
	niazip
)

target_include_directories(NiazipTests PUBLIC
	${PROJECT_SOURCE_DIR}/src/
)

target_sources(NiazipTests PRIVATE
main.cpp
niazip_reader.t.cpp
niazip_writer.t.cpp
 "data/niazip_test_helpers.h")

# Subdirectories
#add_subdirectory(Code)

# Copy data files
add_custom_command(TARGET NiazipTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data)
