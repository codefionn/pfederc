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

add_test(parseline_empty_braces parseline "()")
add_test(parseline_num_braces parseline "(1)")
add_test(parseline_fncall parseline "hello()")

# Test Precedence
add_test(parseline_prec0 parseline "1 + 2")
set_tests_properties(parseline_prec0 PROPERTIES PASS_REGULAR_EXPRESSION
  "^\\(1 \\+ 2\\)\n$")
add_test(parseline_prec1 parseline "1 + 2 * 3")
set_tests_properties(parseline_prec1 PROPERTIES PASS_REGULAR_EXPRESSION
  "^\\(1 \\+ \\(2 \\* 3\\)\\)\n$")
add_test(parseline_prec2 parseline "id0 = id1 = id2")
set_tests_properties(parseline_prec2 PROPERTIES PASS_REGULAR_EXPRESSION
  "^\\(id0 = \\(id1 = id2\\)\\)\n$")
add_test(parseline_prec3 parseline "id0 + id1 + id2")
set_tests_properties(parseline_prec3 PROPERTIES PASS_REGULAR_EXPRESSION
  "^\\(\\(id0 \\+ id1\\) \\+ id2\\)\n$")
