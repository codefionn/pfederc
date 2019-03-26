#include <feder/feder.hpp>
#include <sstream>
using namespace feder;

int main(int argsc, char * argsv[]) {
  if (argsc != 2) return 1;

  std::istringstream input(argsv[1]);
  lexer::Tokenizer lex("<arg>", input);
  lex.nextToken();

  auto prog = parser::parseProgram(lex);
  if (!prog || prog->hasError()) return 1;
  
  return 0;
}
