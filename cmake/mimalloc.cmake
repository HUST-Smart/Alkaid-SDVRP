include(FetchContent)
FetchContent_Declare(
  mimalloc
  GIT_REPOSITORY https://github.com/microsoft/mimalloc
  GIT_TAG v2.0.9)
FetchContent_MakeAvailable(mimalloc)
