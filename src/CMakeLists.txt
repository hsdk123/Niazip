# library declaration
add_library(niazip STATIC)

# library dependencies
target_link_libraries(niazip PUBLIC
	minizip
)

target_include_directories(niazip PUBLIC 
	./
	${minizip_SOURCE_DIR}
)

add_subdirectory(niazip)
