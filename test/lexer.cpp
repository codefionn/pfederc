#include <feder/feder.hpp>
#include <sstream>
using namespace feder::lexer;

int main(int argsc, char * argsv[]) {
  if (argsc == 2) {
    std::istringstream strstream(argsv[1]);

    bool error = false;
    Lexer lexer("<stdin>", strstream);
    Token tok;
    while ((tok = lexer.nextToken()).getType() != tok_eof) {
      if (tok.getType() == tok_err)
        error = true;
      std::cout << std::to_string(tok.getType()) << std::endl;
    }
  
    return error ? EXIT_FAILURE : EXIT_SUCCESS;
  }

  bool error = false;
  Lexer lexer("<stdin>", std::cin);
  Token tok;
  while ((tok = lexer.nextToken()).getType() != tok_eof) {
    if (tok.getType() == tok_err)
      error = true;
    std::cout << std::to_string(tok.getType()) << std::endl;
  }

  return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
