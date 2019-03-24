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

static std::unique_ptr<syntax::StrExpr> _parsePrimaryString(lexer::Lexer &lex) noexcept {
  lexer::Token tok = lex.currentToken();
  lex.nextToken(); // eat str

  return std::make_unique<syntax::StrExpr>(tok.getPosition(),
      tok.getString());
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

  lexer::Token tokcbrace;
  if (!parser::match(lex, &tokcbrace, lexer::tok_cbrace))
    return nullptr;

  lexer::Position pos(posStart, tokcbrace.getPosition());

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

static std::unique_ptr<syntax::Expr> _parsePrimaryArray(lexer::Lexer &lex) noexcept {
  lexer::Position pos = lex.currentToken().getPosition();
  lex.nextToken(); // eat [
}

static std::unique_ptr<syntax::TemplateExpr> _parsePrimaryTemplate(lexer::Lexer &lex) noexcept {
  lexer::Position pos = lex.currentToken().getPosition();
  lex.nextToken(); // eat {
}

static std::unique_ptr<syntax::FuncExpr> _parsePrimaryFunction(lexer::Lexer &lex) noexcept {
   lexer::Position pos = lex.currentToken().getPosition();
  bool virtualFunc = lex.currentToken() == lexer::tok_vfunc;
  lex.nextToken(); // eat 'func'/'Func'
}

static std::vector<std::unique_ptr<syntax::Expr>> _parseInherited(
    lexer::Lexer &lex, bool &error) noexcept {
  error = false;

  if (lex.currentToken() != lexer::tok_op
      || lex.currentToken().getOperator() != lexer::op_tcast)
    return std::vector<std::unique_ptr<syntax::Expr>>();

  lex.nextToken(); // eat ::

  std::vector<std::unique_ptr<syntax::Expr>> result;
  auto expr = parser::parse(lex);
  if (!expr) {
    error = true;
    return std::vector<std::unique_ptr<syntax::Expr>>();
  }

  while (expr->getType() == syntax::expr_biop
      && dynamic_cast<syntax::BiOpExpr&>(*expr).getOperator() == lexer::op_comma) {
    auto &biopexpr = dynamic_cast<syntax::BiOpExpr&>(*expr);

    // , is left associative
    result.push_back(biopexpr.moveRHS());
    expr = biopexpr.moveLHS();
  }

  // The last expr is no biopexpr, so push to result
  result.push_back(std::move(expr));

  return result;
}

static std::unique_ptr<syntax::ClassExpr> _parsePrimaryClass(lexer::Lexer &lex) noexcept {
  lexer::Position pos = lex.currentToken().getPosition();
  lex.nextToken(); // eat 'class'

  lexer::Token idTok;
  if (!parser::match(lex, &idTok, lexer::tok_id))
    return nullptr;

  // deps
  bool errorInherited;
  auto traits = _parseInherited(lex, errorInherited);
  if (errorInherited) return nullptr;

  if (!parser::match(lex, nullptr, lexer::tok_eol)) return nullptr;
}

static std::unique_ptr<syntax::TraitExpr> _parsePrimaryTrait(lexer::Lexer &lex) noexcept {
  lexer::Position pos = lex.currentToken().getPosition();
  lex.nextToken(); // eat 'trait'


  lexer::Token idTok;
  if (!parser::match(lex, &idTok, lexer::tok_id))
    return nullptr;

  // deps
  bool errorInherited;
  auto traits = _parseInherited(lex, errorInherited);
  if (errorInherited) return nullptr;

  if (!parser::match(lex, nullptr, lexer::tok_eol)) return nullptr;
}

static std::unique_ptr<syntax::NmspExpr> _parsePrimaryNamespace(lexer::Lexer &lex) noexcept {
  lexer::Position pos = lex.currentToken().getPosition();
  lex.nextToken(); // eat 'namespace'
  
  lexer::Token tokId;
  if (!parser::match(lex, &tokId, lexer::tok_id))
    return nullptr;

  if (!parser::match(lex, nullptr, lexer::tok_eol))
    return nullptr;

  auto program = parser::parseProgram(lex, false);
  if (program->hasError()) return nullptr;

  if (!parser::match(lex, nullptr, lexer::tok_delim))
    return nullptr;

  return std::make_unique<syntax::NmspExpr>(
      lexer::Position(pos, tokId.getPosition()),
      tokId.getString(),
      std::move(program));
}

std::unique_ptr<syntax::Expr> parser::parsePrimary(lexer::Lexer &lex) noexcept {
  switch (lex.currentToken().getType()) {
  case lexer::tok_id: 
    return _parsePrimaryIdExpr(lex);
  case lexer::tok_num: 
    return _parsePrimaryNumExpr(lex);
  case lexer::tok_str:
    return _parsePrimaryString(lex);
  case lexer::tok_obrace:
    return _parsePrimaryBraceExpr(lex);
  case lexer::tok_obrace_array:
    return _parsePrimaryArray(lex);
  case lexer::tok_obrace_template:
    return _parsePrimaryTemplate(lex);
  case lexer::tok_vfunc:
  case lexer::tok_func:
    return _parsePrimaryFunction(lex);
  case lexer::tok_class:
    return _parsePrimaryClass(lex);
  case lexer::tok_trait:
    return _parsePrimaryTrait(lex);
  case lexer::tok_nmsp:
    return _parsePrimaryNamespace(lex);
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
