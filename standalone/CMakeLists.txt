cmake_minimum_required(VERSION 3.14...3.22)

project(AlkaidSDStandalone LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage("gh:CLIUtils/CLI11@2.3.2")

CPMAddPackage(NAME AlkaidSD SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

# ---- Create standalone executable ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)

add_executable(${PROJECT_NAME} ${sources})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17 OUTPUT_NAME "AlkaidSD")

target_link_libraries(${PROJECT_NAME} AlkaidSD::AlkaidSD CLI11::CLI11)
