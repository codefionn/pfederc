#include "feder/parser.hpp"
using namespace feder;

static std::unique_ptr<syntax::IdExpr> _parsePrimaryIdExpr(lexer::Lexer &lex) noexcept {
  lexer::Token tok = lex.currentToken();
  lex.nextToken(); // eat id

  const std::string &id = tok.getString();
  return std::make_unique<syntax::IdExpr>(tok.getPosition(), id);
}

static std::unique_ptr<syntax::NumExpr> _parsePrimaryNumExpr(lexer::Lexer &lex) noexcept {
  lexer::Token tok = lex.currentToken();
  lex.nextToken(); // eat num

  lexer::NumberType numType = tok.getNumberType();
  lexer::NumberValue numVal = tok.getNumberValue();
  return std::make_unique<syntax::NumExpr>(
      tok.getPosition(), numType, numVal);
}

static std::unique_ptr<syntax::BraceExpr> _parsePrimaryBraceExpr(lexer::Lexer &lex) noexcept {
  lexer::Position posStart = lex.currentToken().getPosition();
  lex.nextToken(); // eat (

  if (lex.currentToken().getType() == lexer::tok_cbrace) {
    // Emtpy brace expression
    lexer::Position pos(posStart, lex.currentToken().getPosition());
    lex.nextToken(); // eat )
    return std::make_unique<syntax::BraceExpr>(pos, nullptr);
  }

  std::unique_ptr<syntax::Expr> expr(parser::parse(lex));
  if (!expr) return nullptr;

  if (lex.currentToken().getType() != lexer::tok_cbrace) {
    syntax::reportSyntaxError(lex, lex.currentToken().getPosition(),
        "Expected token ')'.");
    return nullptr;
  }

  lexer::Position pos(posStart, lex.currentToken().getPosition());
  lex.nextToken(); // eat )

  return std::make_unique<syntax::BraceExpr>(pos, std::move(expr));
}

static std::unique_ptr<syntax::UnOpExpr> _parsePrimaryUnOpExpr(lexer::Lexer &lex) noexcept {
  if (!lexer::isValidOperatorPosition(
        lex.currentToken().getOperator(), lexer::op_lunary)) {
    syntax::reportSyntaxError(lex,
        lex.currentToken().getPosition(),
        "Expected primary token or left-side unary operator.");
    return nullptr;
  }

  lexer::Token opTok = lex.currentToken();
  lex.nextToken(); // eat op
  std::unique_ptr<syntax::Expr> expr(
      parser::parse(lex, opTok.getPrecedence(lexer::op_lunary)));
  if (!expr) return nullptr;

  return std::make_unique<syntax::UnOpExpr>(
      lexer::Position(opTok.getPosition(), expr->getPosition()),
      lexer::op_lunary, opTok.getOperator(), std::move(expr));
}


std::unique_ptr<syntax::Expr> parser::parsePrimary(lexer::Lexer &lex) noexcept {
  switch (lex.currentToken().getType()) {
  case lexer::tok_id: 
    return _parsePrimaryIdExpr(lex);
  case lexer::tok_num: 
    return _parsePrimaryNumExpr(lex);
  case lexer::tok_obrace:
    return _parsePrimaryBraceExpr(lex);
  case lexer::tok_obrace_array:
  case lexer::tok_obrace_template:
  case lexer::tok_func:
  case lexer::tok_class:
  case lexer::tok_trait:
  case lexer::tok_nmsp:
  case lexer::tok_op:
    return _parsePrimaryUnOpExpr(lex);

  case lexer::tok_err:
    return nullptr;
  }

  return syntax::reportSyntaxError(lex,
      lex.currentToken().getPosition(),
      "Not a primary token "
      + std::to_string(lex.currentToken().getType()) + ".");
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
    std::unique_ptr<syntax::Expr> lhs, std::size_t prec) noexcept {

  lexer::Token curtok;
  while (_isBinaryOperator(curtok = lex.currentToken())
      && curtok.getPrecedence() >= prec) {
    lexer::Token opTok = lex.currentToken(); // Operator token
    if (_parseRHSRightUnary(lex, opTok, lhs)) continue;

    if (opTok.getType() == lexer::tok_op) lex.nextToken(); // eat op

    std::unique_ptr<syntax::Expr> rhs(parser::parsePrimary(lex));
    if (!rhs) return nullptr; // Error forwarding

    while (_isBinaryOperator(curtok = lex.currentToken())
        && (curtok.getPrecedence() > opTok.getPrecedence()
          || (curtok.isRightAssociative()
            && curtok.getPrecedence() == opTok.getPrecedence()))) {
      if (_parseRHSRightUnary(lex, curtok, rhs)) continue;
      rhs = parseRHS(lex, std::move(rhs), curtok.getPrecedence());
    }

    lhs = std::make_unique<syntax::BiOpExpr>(
        lexer::Position(lhs->getPosition(), rhs->getPosition()),
        _getOperatorType(opTok), std::move(lhs), std::move(rhs));
  }

  return std::move(lhs);
}

std::unique_ptr<syntax::Expr> parser::parse(lexer::Lexer &lex,
    std::size_t prec) noexcept {
  std::unique_ptr<syntax::Expr> primaryExpr = parser::parsePrimary(lex);
  if (!primaryExpr) return nullptr;

  return parser::parseRHS(lex, std::move(primaryExpr), prec);
}

std::unique_ptr<syntax::Program> parser::parseProgram(lexer::Lexer &lex,
    bool topLevel) noexcept {
  std::vector<std::unique_ptr<syntax::Expr>> lines;

  bool error = false;

  while (true) {
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
