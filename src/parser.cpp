#include "feder/parser.hpp"
using namespace feder;

bool parser::match(lexer::Tokenizer &lex, lexer::Token *tok,
                   lexer::TokenType tokType,
                   lexer::OperatorType opType) noexcept {
  if (lex.currentToken() == tokType &&
      (lex.currentToken() != lexer::tok_op ||
       (lex.currentToken().getOperator() == opType))) {
    if (tok)
      *tok = lex.currentToken();

    if (tokType != lexer::tok_eof)
      lex.nextToken(); // eat current token

    return true; // match successfull
  }

  syntax::reportSyntaxError(lex, lex.currentToken().getPosition(),
                            std::string("Expected token ") +
                                std::to_string(tokType));

  return false; // match not successfull
}

bool parser::match(lexer::Tokenizer &lex, lexer::Token *tok,
                   const std::vector<lexer::TokenType> &tokTypes) noexcept {

  for (auto tokType : tokTypes) {
    if (lex.currentToken() == tokType) {
      if (tok)
        *tok = lex.currentToken();

      if (tokType != lexer::tok_eof)
        lex.nextToken(); // eat matched token

      return true; // match was successfull
    }
  }

  std::string tokStrList;

  // generated token string list for error message
  for (auto it = tokTypes.begin(); it != tokTypes.end(); ++it) {
    if (it != tokTypes.begin())
      tokStrList += ", ";

    tokStrList += std::to_string(*it);
  }

  syntax::reportSyntaxError(lex, lex.currentToken().getPosition(),
                            "Expected one token of " + tokStrList + ".");

  return false;
}

std::vector<std::string>
parser::parseIdentifierCall(lexer::Tokenizer &lex) noexcept {
  lexer::Token idTok;
  if (!parser::match(lex, &idTok, lexer::tok_id))
    return std::vector<std::string>();

  std::vector<std::string> result;
  result.push_back(idTok.getString());
  while (lex.currentToken() == lexer::op_mem) {
    lex.nextToken(); // eat .
    if (!parser::match(lex, &idTok, lexer::tok_id))
      return std::vector<std::string>();

    result.push_back(idTok.getString());
  }

  return result;
}

std::unique_ptr<syntax::Expr>
parser::parseIdentifierCallExpr(lexer::Tokenizer &lex) noexcept {
  lexer::Token idTok;
  if (!parser::match(lex, &idTok, lexer::tok_id))
    return nullptr;

  std::unique_ptr<syntax::Expr> result;
  result =
      std::make_unique<syntax::IdExpr>(lex.getPosition(), idTok.getString());
  while (lex.currentToken() == lexer::op_mem) {
    lex.nextToken(); // eat .
    if (!parser::match(lex, &idTok, lexer::tok_id))
      return nullptr;

    result = std::make_unique<syntax::BiOpExpr>(
        lex.getPosition(), lexer::op_mem, std::move(result),
        std::make_unique<syntax::IdExpr>(lex.getPosition(), idTok.getString()));
  }

  return std::move(result);
}

static bool _isBinaryOperator(const lexer::Token &tok) noexcept {
  switch (tok.getType()) {
  case lexer::tok_op:
  case lexer::tok_caps:
  case lexer::tok_obrace:
  case lexer::tok_obrace_array:
  case lexer::tok_obrace_template:
    return true;
  default:
    return false;
  }
}

static bool _parseRHSRightUnary(lexer::Tokenizer &lex, lexer::Token &opTok,
                                std::unique_ptr<syntax::Expr> &lhs) {
  // Is right-side unary ?
  if (opTok.getType() == lexer::tok_op &&
      isValidOperatorPosition(opTok.getOperator(), lexer::op_runary)) {
    lhs = std::make_unique<syntax::UnOpExpr>(
        lexer::Position(lhs->getPosition(), opTok.getPosition()),
        lexer::op_runary, opTok.getOperator(), std::move(lhs));
    lex.nextToken(); // eat op
    return true;
  }

  return false;
}

static lexer::OperatorType _getOperatorType(lexer::Token &opTok) {
  switch (opTok.getType()) {
  case lexer::tok_caps:
	  return lexer::op_caps;
  case lexer::tok_obrace:
    return lexer::op_fncall;
  case lexer::tok_obrace_array:
    return lexer::op_indexcall;
  case lexer::tok_obrace_template:
    return lexer::op_templatecall;
  case lexer::tok_op:
    return opTok.getOperator();
  default:
    feder::fatal("Reached unreachable code.");
    return lexer::op_sub;
  }
}

std::unique_ptr<syntax::Expr>
parser::parseRHS(lexer::Tokenizer &lex, std::unique_ptr<syntax::Expr> lhs,
                 std::size_t prec, bool parseFunctionDecl) noexcept {

  lexer::Token curtok;

  // https://en.wikipedia.org/wiki/Operator-precedence_parser
  while (_isBinaryOperator(curtok = lex.currentToken()) &&
         curtok.getPrecedence() >= prec) {

    if (parseFunctionDecl && lex.currentToken() == lexer::op_comma)
      break;

    lexer::Token opTok = lex.currentToken(); // Operator token
    if (_parseRHSRightUnary(lex, opTok, lhs))
      continue;

    if (opTok.getType() == lexer::tok_op)
      lex.nextToken(); // eat op

    std::unique_ptr<syntax::Expr> rhs(parser::parsePrimary(lex));
    if (!rhs)
      return nullptr; // Error forwarding

    while (_isBinaryOperator(curtok = lex.currentToken()) &&
           (curtok.getPrecedence() > opTok.getPrecedence() ||
            (curtok.isRightAssociative() &&
             curtok.getPrecedence() == opTok.getPrecedence()))) {

      if (parseFunctionDecl && lex.currentToken() == lexer::op_comma)
        break;

      if (_parseRHSRightUnary(lex, curtok, rhs))
        continue;
      rhs = parseRHS(lex, std::move(rhs), curtok.getPrecedence(),
                     parseFunctionDecl);
    }

    lhs = std::make_unique<syntax::BiOpExpr>(
        lexer::Position(lhs->getPosition(), rhs->getPosition()),
        _getOperatorType(opTok), std::move(lhs), std::move(rhs));
  }

  return std::move(lhs);
}

std::unique_ptr<syntax::Expr> parser::parse(lexer::Tokenizer &lex,
                                            std::size_t prec,
                                            bool parseFunctionDecl) noexcept {
  std::unique_ptr<syntax::Expr> primaryExpr = parser::parsePrimary(lex);
  if (!primaryExpr)
    return nullptr;

  return parser::parseRHS(lex, std::move(primaryExpr), prec, parseFunctionDecl);
}

std::unique_ptr<syntax::Program> parser::parseProgram(lexer::Tokenizer &lex,
                                                      bool topLevel) noexcept {
  std::vector<std::unique_ptr<syntax::Expr>> lines;

  bool error = false;

  while (lex.currentToken() != lexer::tok_eof
      && lex.currentToken() != lexer::tok_delim
      && lex.currentToken() != lexer::tok_else) {

    if (lex.currentToken() == lexer::tok_eol) {
      lex.nextToken();
      continue;
    }

    if (!lexer::isPrimaryToken(lex.currentToken().getType()))
      break;

    std::unique_ptr<syntax::Expr> line(parser::parse(lex));
    if (line && line->isStatement()) {
      lines.push_back(std::move(line));
    } else if (!line) {
      error = true;

      if (!lex.advanced()) {
        error = true;
        break;
      }
    } else {
      error = true;
      lex.reportSemanticError("Expected statement", line->getPosition());

      if (!lex.advanced()) {
        error = true;
        break;
      }
    }

    if (lex.currentToken() != lexer::tok_eof
        && lex.currentToken() != lexer::tok_delim
        && lex.currentToken() != lexer::tok_else) {
      if (!parser::match(lex, nullptr, lexer::tok_eol))
        error = true;
    } else 
      break;
  }

  std::unique_ptr<syntax::Expr> returnExpr;
  if (lex.currentToken() == lexer::tok_return) {
    lex.nextToken(); // eat 'return'
    returnExpr = parser::parse(lex);
    if (!returnExpr)
      error = true;
  }

  // Skip newlines
  while (lex.currentToken() == lexer::tok_eol)
    lex.nextToken();

  if (topLevel) {
    if (!parser::match(lex, nullptr, lexer::tok_eof))
      error = true;
  }

  return std::make_unique<syntax::Program>(std::move(lines),
                                           std::move(returnExpr), error);
}
