include(CTest)
enable_testing()

# lexer
add_executable(lexer "${pfederc_SOURCE_DIR}/test/lexer.cpp")
target_link_libraries(lexer pfederclib)

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
add_test(lexer_num2 lexer "0b10")
set_tests_properties(lexer_num2 PROPERTIES PASS_REGULAR_EXPRESSION "^number\n$")
add_test(lexer_num3 lexer "0o071")
set_tests_properties(lexer_num3 PROPERTIES PASS_REGULAR_EXPRESSION "^number\n$")
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

## Comments
add_test(lexer_comment0 lexer "// i'm a comment")
add_test(lexer_comment1 lexer "/* comment */")

# parseline
add_executable(parseline "${pfederc_SOURCE_DIR}/test/parseline.cpp")
target_link_libraries(parseline pfederclib)

add_test(parseline_num_braces parseline "(1)")
add_test(parseline_fncall parseline "hello()")
add_test(parseline_indexcall parseline "hello[100]")

## Numbers
add_test(parseline_num0 parseline "1")
set_tests_properties(parseline_num0 PROPERTIES PASS_REGULAR_EXPRESSION "^1\n$")
add_test(parseline_num1 parseline "98")
set_tests_properties(parseline_num1 PROPERTIES PASS_REGULAR_EXPRESSION "^98\n$")
add_test(parseline_num2 parseline "1.25")
set_tests_properties(parseline_num2 PROPERTIES PASS_REGULAR_EXPRESSION "^0*1\\.250*\n$")
add_test(parseline_num3 parseline "1.25f")
set_tests_properties(parseline_num3 PROPERTIES PASS_REGULAR_EXPRESSION "^0*1\\.250*\n$")
add_test(parseline_num4 parseline "0xA")
add_test(parseline_num5 parseline "0o12")
add_test(parseline_num6 parseline "0b1010")
set_tests_properties(parseline_num4 parseline_num5 parseline_num6
  PROPERTIES PASS_REGULAR_EXPRESSION "^10\n$")

## Strings
add_test(parseline_str0 parseline "\"Hello world\"")
set_tests_properties(parseline_str0 PROPERTIES PASS_REGULAR_EXPRESSION "^\"Hello world\"\n$")

## Chars
add_test(parseline_char0 parseline "'a'")
set_tests_properties(parseline_char0 PROPERTIES PASS_REGULAR_EXPRESSION "^'a'\n$")

## Skip new line
add_test(parseline_skipline0 parseline "(\n 1 + \n 2)")
add_test(parseline_skipline1 parseline "func hello(\nstr : std.String) ;")
add_test(parseline_skipline2 parseline "func hello(str \n : std.String) ;")

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

add_test(parseline_prec4 parseline "!std.name.hello")
set_tests_properties(parseline_prec4 PROPERTIES PASS_REGULAR_EXPRESSION
  "^\\(!\\(\\(std . name\\) . hello\\)\\)\n$")
add_test(parseline_prec5 parseline "+std.name.hello")
set_tests_properties(parseline_prec5 PROPERTIES PASS_REGULAR_EXPRESSION
  "^\\(\\+\\(\\(std . name\\) . hello\\)\\)\n$")
add_test(parseline_prec6 parseline "-std.name.hello")
set_tests_properties(parseline_prec6 PROPERTIES PASS_REGULAR_EXPRESSION
  "^\\(-\\(\\(std . name\\) . hello\\)\\)\n$")
add_test(parseline_prec7 parseline "~std.name.hello")
set_tests_properties(parseline_prec7 PROPERTIES PASS_REGULAR_EXPRESSION
  "^\\(~\\(\\(std . name\\) . hello\\)\\)\n$")

add_test(parseline_runary0 parseline "i--")
set_tests_properties(parseline_runary0 PROPERTIES PASS_REGULAR_EXPRESSION
  "^\\(i--\\)\n$")
add_test(parseline_runary1 parseline "i++")
set_tests_properties(parseline_runary1 PROPERTIES PASS_REGULAR_EXPRESSION
  "^\\(i\\+\\+\\)\n$")
add_test(parseline_runary2 parseline "i++ + 2")
set_tests_properties(parseline_runary2 PROPERTIES PASS_REGULAR_EXPRESSION
  "^\\(\\(i\\+\\+\\) \\+ 2\\)\n$")

add_test(parseline_runary3 parseline "i + + 2")
set_tests_properties(parseline_runary3 PROPERTIES PASS_REGULAR_EXPRESSION
  "^\\(i \\+ \\(\\+2\\)\\)\n$")


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

## Functions - Fail tests
add_test(parseline_ffn00 parseline "func ;")
add_test(parseline_ffn01 parseline "func{T}")
add_test(parseline_ffn02 parseline "Func (t : T)")
add_test(parseline_ffn03 parseline "func hello(t : T)")
add_test(parseline_ffn04 parseline "func hello(t : T) : T")
add_test(parseline_ffn05 parseline "func hello(t : T) : T\n")
add_test(parseline_ffn06 parseline "func std.hello(t : T) ;")
set_tests_properties(parseline_ffn00 parseline_ffn01 parseline_ffn02
  parseline_ffn03 parseline_ffn04 parseline_ffn05 parseline_ffn06
  PROPERTIES WILL_FAIL TRUE)

## Traits
add_test(parseline_trait0 parseline "trait T\nFunc hello ;\n;")
add_test(parseline_trait1 parseline "trait T :: MyTrait0, MyTrait1\nFunc hello ;\n;")
add_test(parseline_trait2 parseline "trait T :: MyTrait0, MyTrait1\nFunc hello1 ;\n Func hello1 ;\n;")
add_test(parseline_trait3 parseline "trait{T} MyTrait :: MyTraitParent\n;")
add_test(parseline_trait4 parseline "trait MyTrait\nFunc hello : u8 ;\n;")

## Traits - Fail tests
add_test(parseline_ftrait0 parseline "trait T\n;")
add_test(parseline_ftrait1 parseline "trait T\nFunc hello0 ; Func hello1\n;")
add_test(parseline_ftrait2 parseline "trait T Func hello0;\n Func hello1\n;")
add_test(parseline_ftrait3 parseline "trait T\n func hello\n;")
set_tests_properties(parseline_ftrait0 parseline_ftrait1
  parseline_ftrait2 parseline_ftrait3
  PROPERTIES WILL_FAIL TRUE)

## Classes

add_test(parseline_class0 parseline "class Str\nvar : MyClass\nfunc Str : Str\ndoSomething()\n;\nfunc hello0 ;\nFunc hello\ndoSomething()\n;\n;")
add_test(parseline_class1 parseline "class Str :: MyClass\nvar : MyClass\nfunc Str : Str\ndoSomething()\n;\nfunc hello0 ;\nfunc hello\ndoSomething()\n;\n;")
add_test(parseline_class2 parseline "class Str :: MyClass\nvar0 : MyClass\nfunc _Str : Str\ndoSomething()\n;\nfunc Str : Str\ndoSomething()\n;\nfunc hello0 ;\nfunc hello\ndoSomething()\n;\nvar1 : MyClass\n;")

## Classes - Fail tests

add_test(parseline_fclass0 parseline "class Str")
add_test(parseline_fclass1 parseline "class Str\nFunc Str : Str ;\n;")
add_test(parseline_fclass2 parseline "class Str\nfunc my.function ;\n;")
set_tests_properties(parseline_fclass0 parseline_fclass1 parseline_fclass2
  PROPERTIES WILL_FAIL TRUE)

## Enums

add_test(parseline_enum0 parseline "enum Weekday\nMonday\nTuesday\nWednesday\nAnother\n;")
add_test(parseline_enum1 parseline "enum{T} Option\nNothing\n;")
add_test(parseline_enum2 parseline "enum{T} Option\nNothing\nJust(T)\n;")

## Enum - Fail tests

add_test(parseline_fenum0 parseline "enum Weekday\nMonday + Tuesday\n;")
add_test(parseline_fenum1 parseline "enum Weekday\n1(T)\n;")
add_test(parseline_fenum2 parseline "enum Weekday\n1\n;")
set_tests_properties(parseline_fenum0 parseline_fenum1 parseline_fenum2
  PROPERTIES WILL_FAIL TRUE)

## Arrays

add_test(parseline_array0 parseline "[\"hello\" ; 100]")
add_test(parseline_array1 parseline "[\"hello0\", \"hello1\"]")
add_test(parseline_array2 parseline "hello[100]")
add_test(parseline_array3 parseline "hello[100 + 12354]")
add_test(parseline_array4 parseline "hello[(100 + 12354)]")

## Templates
add_test(parseline_templ0 parseline "hello{myawesometype}")
add_test(parseline_templ1 parseline "func{T : myawesometype} hello(t : T) ;")
add_test(parseline_templ2 parseline "hello{mytype0, mytype1, mytype2}")
add_test(parseline_templ3 parseline "func{t0 : T, t1 :T} hello() ;")

## If
add_test(parseline_if0 parseline "if true\nretfalse()\n;")
add_test(parseline_if1 parseline "if true\nrettrue()\nelse\nretfalse()\n;")
add_test(parseline_if2 parseline "if i == 0\nhello1()\nelse if i == 1\nhello2()\nelse\nhello3()\n;")
add_test(parseline_if3 parseline "if i == 0\nhello1()\nelse if i == 1\nhello2()\nelse if i == 2\nhello3()\nelse\nhello4()\n;")
add_test(parseline_if4 parseline "if i == 0\nhello1()\nelse if i == 1\nhello2()\n;")

## If - Fail tests
add_test(parseline_fif0 parseline "if else ;")
add_test(parseline_fif1 parseline "if\nelse\n;")
add_test(parseline_fif2 parseline "if true rettrue()\n;")
add_test(parseline_fif3 parseline "if true\nrettrue()\nelse retfalse()\n;")
set_tests_properties(parseline_fif0 parseline_fif1 parseline_fif2
  parseline_fif3
  PROPERTIES WILL_FAIL TRUE)

## Match
add_test(parseline_match0 parseline "match enumVal\nenumCase => doSomething()\n;\n;")
add_test(parseline_match1 parseline "match enumVal\nenumCase1 => doSmth0()\n;\nenumCase1 => doSmth1()\n;\n;")
add_test(parseline_match2 parseline "match enumVal\nenumCase1 => doSmth0()\n;\nenumCase1 => doSmth1()\n;\nenumCase2 => doSmth2()\n;\n;")
add_test(parseline_match3 parseline "match enumVal\nenumCase(x) => doSmth0()\n;\n;")
add_test(parseline_match4 parseline "match enumVal\nenumCase(x, y) => doSmth0()\n;\n;")
add_test(parseline_match5 parseline "match enumVal\nenumCase => ;\n;")
add_test(parseline_match6 parseline "match enumVal\nmy.enumCase => doSmth()\n;\n;")
add_test(parseline_match7 parseline "match enumVal\nnot.my.enumCase => doSmth()\n;\n;")

## Match - Fail tests
add_test(parseline_fmatch0 parseline "match enumVal enumCase => doSomethings()\n;\n;")
add_test(parseline_fmatch1 parseline "match enumVal\n enumCase => doSomethings()\n;;")
add_test(parseline_fmatch2 parseline "match enumVal\n enumCase")
add_test(parseline_fmatch3 parseline "match enumVal\n;")
set_tests_properties(parseline_fmatch0 parseline_fmatch1 parseline_fmatch2
  parseline_fmatch3
  PROPERTIES WILL_FAIL TRUE)

## Comments
add_test(parseline_comment0 parseline "func{T} hello(t : T) // comment\n;")
add_test(parseline_comment1 parseline "/* 111 == */ 100 + 11")
add_test(parseline_comment2 parseline "2 + /* 6 == */ 2 * 3")

# parseinput
add_executable(parseinput "${pfederc_SOURCE_DIR}/test/parseinput.cpp")
target_link_libraries(parseinput pfederclib)

# parseprogram
add_executable(parseprogram "${pfederc_SOURCE_DIR}/test/parseprogram.cpp")
target_link_libraries(parseprogram pfederclib)

add_test(parseprogram_decl0 parseprogram "var0 : T\nvar1 : T")
add_test(parseprogram_decl1 parseprogram "var0 : T\nvar1 : T\nvar2 : T")

add_test(parseprogram_funcs0 parseprogram "t0 : func\nt1 : func")
add_test(parseprogram_funcs1 parseprogram "t0 : func : T\nt1 : func : T")

add_test(parseprogram_class0 parseprogram "class MyClass \nfunc hello ;\n;")
add_test(parseprogram_nmsp0 parseprogram "namespace std\n;")
add_test(parseprogram_enum0 parseprogram "enum Weekday\nMonday\nTuesday\n;")
add_test(parseprogram_func0 parseprogram "func hello ;")
add_test(parseprogram_func1 parseprogram "func hello \n;")
add_test(parseprogram_trait0 parseprogram "trait MyTrait \n Func hello ; \n;")

## return

add_test(parseprogram_ret0 parseprogram "return true")
add_test(parseprogram_ret1 parseprogram "if true\nreturn true\n;")

## Program - Fail tests
add_test(parseprogram_fail0 parseprogram "1 + 2")
add_test(parseprogram_fail1 parseprogram "hello[1]")
add_test(parseprogram_fail2 parseprogram "func _ \n;")
add_test(parseprogram_fail3 parseprogram "class _\nfunc hello ;\n;")
add_test(parseprogram_fail4 parseprogram "func")
add_test(parseprogram_fail5 parseprogram "hello{MyClass}")
add_test(parseprogram_fail6 parseprogram "return true\nreturn false\n")
set_tests_properties(parseprogram_fail0 parseprogram_fail1
  parseprogram_fail2 parseprogram_fail3 parseprogram_fail4
  parseprogram_fail5 parseprogram_fail6
  PROPERTIES WILL_FAIL TRUE)

# Valgrind (memory check)
#| For better valgrind stack-traces pass '-DCMAKE_BUILD_TYPE=Debug',
#| when generating cmake build cache.

option(PFEDERC_TEST_VALGRIND "Test for memory leaks with valgrind" ON)

find_program(VALGRIND "valgrind")
if (VALGRIND AND PFEDERC_TEST_VALGRIND)
  set(VALGRIND_PROG ${VALGRIND} "--tool=memcheck" "--leak-check=yes")

  add_test(NAME valgrind_parseline0 COMMAND ${VALGRIND_PROG} $<TARGET_FILE:parseline> "hello[100]")
  add_test(NAME valgrind_parseline1 COMMAND ${VALGRIND_PROG} $<TARGET_FILE:parseline> "trait MyTrait :: MyTraitParent\nFunc helloWorld : std.String ; \n;")
  add_test(NAME valgrind_parseline2 COMMAND ${VALGRIND_PROG} $<TARGET_FILE:parseline> "func helloWorld\nstd.io.println(\"Hello, World!\")\n;")
  add_test(NAME valgrind_parseprogram0 COMMAND ${VALGRIND_PROG} $<TARGET_FILE:parseprogram> "std.io.print(\"Hello, \")\nstd.io.println(\"World!\")")
endif()
