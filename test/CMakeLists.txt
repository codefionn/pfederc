include(CTest)
enable_testing()

# Build test libary (consume less resources)
add_library(pfederctestlib STATIC ${pfederc_SOURCES})
if (NOT PFEDERC_SANITY_CHECKS)
  target_compile_definitions(pfederctestlib PRIVATE NSANITY=1)
endif()

# lexer
add_executable(lexer "${pfederc_SOURCE_DIR}/test/lexer.cpp")
target_link_libraries(lexer pfederctestlib)

add_test(lexer_add lexer "2 + id\n")
set_tests_properties(lexer_add PROPERTIES PASS_REGULAR_EXPRESSION
  "^number\noperator\nidentifier\nend-of-line\n$")

# Check if all existing lines are read
add_test(lexer_lines lexer "id0\nid1\nid2\n\n")
set_tests_properties(lexer_lines PROPERTIES PASS_REGULAR_EXPRESSION
  "^identifier\nend-of-line\nidentifier\nend-of-line\nidentifier\nend-of-line\nend-of-line\n$")

## Numbers
add_test(lexer_num0 lexer "0x0F")
set_tests_properties(lexer_num0 PROPERTIES PASS_REGULAR_EXPRESSION "^number\n$")
add_test(lexer_num1 lexer "10")
set_tests_properties(lexer_num1 PROPERTIES PASS_REGULAR_EXPRESSION "^number\n$")
#add_test(lexer_num2 lexer "0b10")
#set_tests_properties(lexer_num2 PROPERTIES PASS_REGULAR_EXPRESSION "^number\n$")
#add_test(lexer_num3 lexer "0o071")
#set_tests_properties(lexer_num3 PROPERTIES PASS_REGULAR_EXPRESSION "^number\n$")
add_test(lexer_num4 lexer "10.0")
set_tests_properties(lexer_num4 PROPERTIES PASS_REGULAR_EXPRESSION "^number\n$")
add_test(lexer_num5 lexer "0.0")
set_tests_properties(lexer_num5 PROPERTIES PASS_REGULAR_EXPRESSION "^number\n$")
add_test(lexer_num6 lexer "0.10")
set_tests_properties(lexer_num6 PROPERTIES PASS_REGULAR_EXPRESSION "^number\n$")
add_test(lexer_num7 lexer "0.21")
set_tests_properties(lexer_num7 PROPERTIES PASS_REGULAR_EXPRESSION "^number\n$")
add_test(lexer_num8 lexer "0s")
set_tests_properties(lexer_num8 PROPERTIES PASS_REGULAR_EXPRESSION "^number\n$")
add_test(lexer_num9 lexer "1s")
set_tests_properties(lexer_num9 PROPERTIES PASS_REGULAR_EXPRESSION "^number\n$")

## Strings
add_test(lexer_str0 lexer "\"Hello, World!\"")
set_tests_properties(lexer_str0 PROPERTIES PASS_REGULAR_EXPRESSION "^string\n$")
add_test(lexer_str1 lexer "\"\\t\\n\"")
set_tests_properties(lexer_str1 PROPERTIES PASS_REGULAR_EXPRESSION "^string\n$")
add_test(lexer_str2 lexer "\"\"")
set_tests_properties(lexer_str2 PROPERTIES PASS_REGULAR_EXPRESSION "^string\n$")

## Chars
add_test(lexer_char0 lexer "' '")
set_tests_properties(lexer_char0 PROPERTIES PASS_REGULAR_EXPRESSION "^character\n$")

## Check if operators get parsed
add_test(lexer_operators lexer
  "++ -- += + -> -= &= && & |= || | *= * != ! === == = :: :? := : <<= << <= < >>= >> >= > ~ ^= ^ /= / %= % . ,")
set_tests_properties(lexer_operators PROPERTIES PASS_REGULAR_EXPRESSION
  "^operator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\noperator\n$")


# parseline
add_executable(parseline "${pfederc_SOURCE_DIR}/test/parseline.cpp")
target_link_libraries(parseline pfederctestlib)

add_test(parseline_num_braces parseline "(1)")
add_test(parseline_fncall parseline "hello()")
add_test(parseline_indexcall parseline "hello[100]")

## Numbers
add_test(parseline_num0 parseline "1")
set_tests_properties(parseline_num0 PROPERTIES PASS_REGULAR_EXPRESSION "^1\n$")

## Skip new line
add_test(parseline_skipline parseline "(\n 1 + \n 2)")

## Test Precedence
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

## Functions
add_test(parseline_fn00 parseline "func hello ;")
add_test(parseline_fn01 parseline "func hello : hellot ;")
add_test(parseline_fn02 parseline "func hello(param : hellot) : hellot ;")
add_test(parseline_fn03 parseline "func hello(param0 : hellot0, param1 : hellot1) : hellot ;")
add_test(parseline_fn04 parseline "func hello(param0 : hellot0, param1 : hellot1, param2 : hellot2) : hellot ;")
add_test(parseline_fn05 parseline "func hello\nhello()\n;")
add_test(parseline_fn06 parseline "func hello : hellot\nhello(helloval)\n;")
add_test(parseline_fn07 parseline "func hello(param : hellot) : hellot\nhello(param)\n;")
add_test(parseline_fn07 parseline "func hello(param0 : hellot0, param1 : hellot1) : hellot\nhello(param0)\nhello(param1)\n;")
add_test(parseline_fn08 parseline "func")
add_test(parseline_fn09 parseline "func : hellot")
add_test(parseline_fn10 parseline "func (param : hellot) : hellot")
add_test(parseline_fn11 parseline "func (param0 : hellot0, param1 : hellot1) : hellot")

## Arrays

add_test(parseline_array0 parseline "[\"hello\" ; 100]")
add_test(parseline_array1 parseline "[\"hello0\", \"hello1\"]")
add_test(parseline_array2 parseline "hello[100]")
add_test(parseline_array3 parseline "hello[100 + 12354]")
add_test(parseline_array4 parseline "hello[(100 + 12354)]")

add_test(parseline_templ0 parseline "hello{myawesometype}")
add_test(parseline_templ1 parseline "func{T : myawesometype} hello(t : T) ;")

# parseinput
add_executable(parseinput "${pfederc_SOURCE_DIR}/test/parseinput.cpp")
target_link_libraries(parseinput pfederctestlib)
