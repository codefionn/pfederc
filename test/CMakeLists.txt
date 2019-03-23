include(CTest)
enable_testing()

# Build test libary (consume less resources)
add_library(pfederctestlib STATIC ${pfederc_SOURCES})

# lexer
add_executable(lexer "${pfederc_SOURCE_DIR}/test/lexer.cpp")
target_link_libraries(lexer pfederctestlib)

add_test(lexer_add lexer "2 + id\n")
set_tests_properties(lexer_add PROPERTIES PASS_REGULAR_EXPRESSION
  "number\noperator\nidentifier\nend-of-line\n$")

# Check if all existing lines are read
add_test(lexer_lines lexer "id0\nid1\nid2\n\n")
set_tests_properties(lexer_lines PROPERTIES PASS_REGULAR_EXPRESSION
  "identifier\nend-of-line\nidentifier\nend-of-line\nidentifier\nend-of-line\nend-of-line\n$")

# Check if operators get parsed
add_test(lexer_operators lexer
  "++ -- += + -> -= &= && & |= || | *= * != ! === == = :: :? := : <<= << <= < >>= >> >= > ~ ^= ^ /= / %= % . ,")
set_tests_properties(lexer_operators PROPERTIES PASS_REGULAR_EXPRESSION
  "operator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\n$")


# parseline
add_executable(parseline "${pfederc_SOURCE_DIR}/test/parseline.cpp")
target_link_libraries(parseline pfederctestlib)
