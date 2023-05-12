cmake_minimum_required(VERSION 3.14)
project(AlkaidSD)

set(CMAKE_CXX_STANDARD 20)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -march=native")
  if (PGO STREQUAL "generate")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-instr-generate=${CMAKE_SOURCE_DIR}/pgo-clang/dimacs-sd-%p.profraw")
  endif ()
  if (PGO STREQUAL "use")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-instr-use=${CMAKE_SOURCE_DIR}/pgo-clang/dimacs-sd.profdata")
  endif ()
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -fuse-ld=lld")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ofast -march=native")
  if (PGO STREQUAL "generate")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-generate=${CMAKE_SOURCE_DIR}/pgo-gcc/dimacs-sd-%q{PROFILE_FILE}")
  endif ()
  if (PGO STREQUAL "use")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fprofile-use=${CMAKE_SOURCE_DIR}/pgo-gcc/dimacs-sd")
  endif ()
endif()

include(cmake/cli11.cmake)
include(cmake/json.cmake)
include(cmake/mimalloc.cmake)

include_directories(src)

add_library(
  alkaid-sd
  src/algorithm/construction.cc
  src/algorithm/operator/cross.cc
  src/algorithm/operator/exchange.cc
  src/algorithm/operator/or_opt.cc
  src/algorithm/operator/relocate.cc
  src/algorithm/operator/repair.cc
  src/algorithm/operator/sd_swap_one_one.cc
  src/algorithm/operator/sd_swap_star.cc
  src/algorithm/operator/sd_swap_two_one.cc
  src/algorithm/operator/split_reinsertion.cc
  src/algorithm/operator/swap_star.cc
  src/algorithm/operator/swap.cc
  src/algorithm/ruin_method.cc
  src/algorithm/solver.cc
  src/algorithm/sorter.cc
  src/config/config.cc
  src/distance_matrix.cc
  src/model/route_context.cc
  src/problem.cc
  src/util/solution_utils.cc
  src/util/submit.cc
  src/util/timer.cc)

target_link_libraries(alkaid-sd nlohmann_json::nlohmann_json)

add_executable(alkaid-sd-main src/main.cc)

target_link_libraries(alkaid-sd-main PRIVATE alkaid-sd CLI11::CLI11 mimalloc-static)
