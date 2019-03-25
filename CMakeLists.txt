cmake_minimum_required(VERSION 3.0)
project(pfederc)

include_directories("${pfederc_SOURCE_DIR}/include")

option(PFEDERC_SANITY_CHECKS "Do sanity checks" OFF)

set(pfederc_SOURCES
  "${pfederc_SOURCE_DIR}/src/lexer.cpp"
  "${pfederc_SOURCE_DIR}/src/global.cpp"
  "${pfederc_SOURCE_DIR}/src/syntax.cpp"
  "${pfederc_SOURCE_DIR}/src/parser.cpp"
  "${pfederc_SOURCE_DIR}/src/parser_primary.cpp")

# pfederclib
add_library(pfederclib ${pfederc_SOURCES})
set_property(TARGET pfederclib PROPERTY CXX_STANDARD 14)

# pfederclib - compile features

target_compile_features(pfederclib PUBLIC cxx_std_14)

# pfederclib - sanity checks
if (NOT PFEDERC_SANITY_CHECKS)
  target_compile_definitions(pfederclib PRIVATE NSANITY=1)
endif()

add_executable(pfederc "${pfederc_SOURCE_DIR}/src/main.cpp")
target_link_libraries(pfederc pfederclib)

add_subdirectory("${pfederc_SOURCE_DIR}/test")
