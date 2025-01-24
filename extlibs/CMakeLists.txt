# Build external dependencies
include_directories(./)

include(FetchContent)

# minizip
# MZ_FORCE_FETCH_LIBS: always force fetch deps for minizip to prevent headaches.
set(MZ_FORCE_FETCH_LIBS ON CACHE BOOL "" FORCE)

# remove unneeded compression methods
# reference: https://github.com/zlib-ng/minizip-ng/blob/master/mz_zip_rw.c#L1913
set(MZ_BZIP2 OFF CACHE BOOL "" FORCE)
set(MZ_LZMA OFF CACHE BOOL "" FORCE)
set(MZ_ZSTD OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  minizip
  #GIT_REPOSITORY https://github.com/zlib-ng/minizip-ng
  #GIT_TAG 03e4d2213d2e3522dfbdbb6b13d0d293b6fa2569 # using this due to https://github.com/zlib-ng/minizip-ng/issues/692
  
  GIT_REPOSITORY https://github.com/hsdk123/minizip.git
  GIT_TAG        master
)
FetchContent_MakeAvailable(minizip)

if (WITH_NIAZIP_TESTS)

# google test
# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(googletest)

endif()
