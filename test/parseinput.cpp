#include <feder/feder.hpp>
#include <sstream>
using namespace feder;

int main(int argsc, char * argsv[]) {
  lexer::Lexer lex("<stdin>", std::cin);
  lex.nextToken();

  auto line = parser::parse(lex);
  if (!line) return 1;

  if (lex.currentToken() != lexer::tok_eof) {
    std::cerr << "After parsing one expression: EOF not reached!" << std::endl;
    std::cerr << "Current token: " << std::to_string(lex.currentToken().getType()) << std::endl;
    return 1;
  }

  std::cout << line->to_string() << std::endl;
  return 0;
}
