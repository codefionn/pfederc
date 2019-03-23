#include <feder/feder.hpp>
#include <sstream>
using namespace feder;

int main(int argsc, char * argsv[]) {
  lexer::Lexer lex("<stdin>", std::cin);
  lex.nextToken();

  auto line = parser::parse(lex);
  if (!line) return 1;

  std::cout << line->to_string() << std::endl;
  return 0;
}
