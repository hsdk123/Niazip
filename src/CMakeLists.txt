# library declaration
add_library(Niazip STATIC)

# library dependencies
target_link_libraries(Niazip PUBLIC
	minizip
)

target_include_directories(Niazip PUBLIC 
	./
	${minizip_SOURCE_DIR}
)

add_subdirectory(niazip)
