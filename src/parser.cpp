#include "feder/parser.hpp"
using namespace feder;

bool parser::match(lexer::Lexer &lex,
    lexer::Token *tok,
    lexer::TokenType tokType, lexer::OperatorType opType) noexcept {
  if (lex.currentToken() == tokType
      && (lex.currentToken() != lexer::tok_op
        || (lex.currentToken().getOperator() == opType))) {
    if (tok)
      *tok = lex.currentToken();

    lex.nextToken(); // eat current token

    return true; // match successfull
  }

  syntax::reportSyntaxError(lex,
      lex.currentToken().getPosition(),
      std::string("Expected token ") + std::to_string(tokType));

  return false; // match not successfull
}

static bool _isRightSideUnary(lexer::Token &tokOp, lexer::Lexer &lex) noexcept {
  if (!lexer::isPrimaryToken(lex.currentToken().getType())) return true;

  if (lex.currentToken().getType() == lexer::tok_op) {
    if (!lexer::isValidOperatorPosition(tokOp.getOperator(), lexer::op_binary))
      return true;

    if (!lexer::isValidOperatorPosition(tokOp.getOperator(), lexer::op_runary))
      return false;

    if (!lexer::isValidOperatorPosition(lex.currentToken().getOperator(),
          lexer::op_lunary))
      return true;
  }

  return false;
}

static bool _isBinaryOperator(const lexer::Token &tok) noexcept {
  switch (tok.getType()) {
    case lexer::tok_op:
    case lexer::tok_obrace:
    case lexer::tok_obrace_array:
    case lexer::tok_obrace_template:
      return true;
    default:
      return false;
  }
}

static bool _parseRHSRightUnary(lexer::Lexer &lex, lexer::Token &opTok,
  std::unique_ptr<syntax::Expr> &lhs) {
  // Is right-side unary ?
  if (opTok.getType() == lexer::tok_op
      && isValidOperatorPosition(opTok.getOperator(), lexer::op_runary)) {
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
  case lexer::tok_obrace:
    return lexer::op_fncall;
  case lexer::tok_obrace_array:
    return lexer::op_indexcall;
  case lexer::tok_op:
    return opTok.getOperator();
  }

  feder::fatal("Reached unreachable code.");
  return lexer::op_sub;
}

std::unique_ptr<syntax::Expr> parser::parseRHS(lexer::Lexer &lex,
    std::unique_ptr<syntax::Expr> lhs, std::size_t prec,
        bool parseFunctionDecl) noexcept {

  lexer::Token curtok;

  // https://en.wikipedia.org/wiki/Operator-precedence_parser
  while (_isBinaryOperator(curtok = lex.currentToken())
      && curtok.getPrecedence() >= prec) {

    if (parseFunctionDecl && lex.currentToken() == lexer::op_comma) {
      break;
    }

    lexer::Token opTok = lex.currentToken(); // Operator token
    if (_parseRHSRightUnary(lex, opTok, lhs)) continue;

    if (opTok.getType() == lexer::tok_op) lex.nextToken(); // eat op

    std::unique_ptr<syntax::Expr> rhs(parser::parsePrimary(lex));
    if (!rhs) return nullptr; // Error forwarding

    while (_isBinaryOperator(curtok = lex.currentToken())
        && (curtok.getPrecedence() > opTok.getPrecedence()
          || (curtok.isRightAssociative()
            && curtok.getPrecedence() == opTok.getPrecedence()))) {

      if (parseFunctionDecl && lex.currentToken() == lexer::op_comma) {
        break;
      }

      if (_parseRHSRightUnary(lex, curtok, rhs)) continue;
      rhs = parseRHS(lex, std::move(rhs), curtok.getPrecedence(),
          parseFunctionDecl);
    }

    lhs = std::make_unique<syntax::BiOpExpr>(
        lexer::Position(lhs->getPosition(), rhs->getPosition()),
        _getOperatorType(opTok), std::move(lhs), std::move(rhs));
  }

  return std::move(lhs);
}

std::unique_ptr<syntax::Expr> parser::parse(lexer::Lexer &lex,
    std::size_t prec,
    bool parseFunctionDecl) noexcept {
  std::unique_ptr<syntax::Expr> primaryExpr = parser::parsePrimary(lex);
  if (!primaryExpr) return nullptr;

  return parser::parseRHS(lex, std::move(primaryExpr), prec,
      parseFunctionDecl);
}

std::unique_ptr<syntax::Program> parser::parseProgram(lexer::Lexer &lex,
    bool topLevel) noexcept {
  std::vector<std::unique_ptr<syntax::Expr>> lines;

  bool error = false;

  while (true) {
    if (!lexer::isPrimaryToken(lex.currentToken().getType()))
      break;

    std::unique_ptr<syntax::Expr> line(parser::parse(lex));
    if (line)
      lines.push_back(std::move(line));
    else
      error = true;

    if (lex.currentToken() == lexer::tok_eof)
      break;

    lex.nextToken();
  }

  if (topLevel && lex.currentToken() != lexer::tok_eof) {
    syntax::reportSyntaxError(lex,
        lex.currentToken().getPosition(),
        "Expected end-of-file.");
    return nullptr;
  }

  return std::make_unique<syntax::Program>(std::move(lines), error);
}
