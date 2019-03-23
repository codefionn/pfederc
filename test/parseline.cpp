#include <feder/feder.hpp>
#include <sstream>
using namespace feder;

int main(int argsc, char * argsv[]) {
  if (argsc != 2)
    return 1;

  std::istringstream input(argsv[1]);
  lexer::Lexer lex("<arg>", input);
  lex.nextToken();

  auto line = parser::parse(lex);
  if (!line) return 1;

  std::cout << line->to_string() << std::endl;
  return 0;
}
