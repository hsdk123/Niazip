# Build external dependencies
include_directories(./)

include(FetchContent)

# minizip
FetchContent_Declare(
  minizip
  GIT_REPOSITORY https://github.com/nmoinvaz/minizip.git
  #GIT_REPOSITORY https://github.com/hsdk123/minizip.git
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
