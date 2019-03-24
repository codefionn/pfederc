cmake_minimum_required(VERSION 3.0)
project(pfederc)

include_directories("${pfederc_SOURCE_DIR}/include")

set(pfederc_SOURCES
  "${pfederc_SOURCE_DIR}/src/lexer.cpp"
  "${pfederc_SOURCE_DIR}/src/global.cpp"
  "${pfederc_SOURCE_DIR}/src/syntax.cpp"
  "${pfederc_SOURCE_DIR}/src/parser.cpp"
  "${pfederc_SOURCE_DIR}/src/parser_primary.cpp")

add_executable(pfederc ${pfederc_SOURCES}
  "${pfederc_SOURCE_DIR}/src/main.cpp")

add_subdirectory("${pfederc_SOURCE_DIR}/test")
