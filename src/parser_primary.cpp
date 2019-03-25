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
  lex.skipNewLine = true;

  lexer::Position posStart = lex.currentToken().getPosition();
  lex.nextToken(); // eat (

  if (lex.currentToken().getType() == lexer::tok_cbrace) {
    // Emtpy brace expression
    lexer::Position pos(posStart, lex.currentToken().getPosition());
    lex.skipNewLine = false;
    lex.nextToken(); // eat )
    return std::make_unique<syntax::BraceExpr>(pos, nullptr);
  }

  std::unique_ptr<syntax::Expr> expr(parser::parse(lex));
  if (!expr) return nullptr;

  lex.skipNewLine = false;

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
  lexer::Position posStart = lex.currentToken().getPosition();
  lex.nextToken(); // eat [

  auto expr = parser::parse(lex);
  if (!expr) return nullptr; // error forwarding

  if (lex.currentToken() == lexer::tok_delim) {
    // Constructor
    lex.nextToken(); // eat ;
    auto expr1 = parser::parse(lex);
    if (!expr1) return nullptr; // error forwarding

    lexer::Token posEndTok;
    if (!parser::match(lex, &posEndTok, lexer::tok_cbrace_array)) return nullptr;

    return std::make_unique<syntax::ArrayConExpr>(
        lexer::Position(posStart, posEndTok.getPosition()),
        std::move(expr), std::move(expr1));
  }

  lexer::Token posEndTok;
  if (!parser::match(lex, &posEndTok, lexer::tok_cbrace_array)) return nullptr;

  if (expr->getType() == syntax::expr_biop
      && dynamic_cast<syntax::BiOpExpr&>(*expr).getOperator() == lexer::op_comma) {
    // List
    std::vector<std::unique_ptr<syntax::Expr>> objs;
    // Iterator through LHSs of expr and append RHS to objs
    while (expr->getType() == syntax::expr_biop
      && dynamic_cast<syntax::BiOpExpr&>(*expr).getOperator() == lexer::op_comma) {
      auto &biopexpr = dynamic_cast<syntax::BiOpExpr&>(*expr);

      // Comma is left-associative binary operator.
      objs.push_back(biopexpr.moveRHS());
      
      expr = biopexpr.moveLHS(); // iterator step
    }

    // Last element
    objs.push_back(std::move(expr));

    return std::make_unique<syntax::ArrayListExpr>(
        lexer::Position(posStart, posEndTok.getPosition()), std::move(objs));
  }

  // Index expression
  return std::make_unique<syntax::ArrayIndexExpr>(
      lexer::Position(posStart, posEndTok.getPosition()), std::move(expr));
}

static std::unique_ptr<syntax::TemplateExpr> _parsePrimaryTemplate(lexer::Lexer &lex) noexcept {
  lexer::Position startPos = lex.currentToken().getPosition();
  lex.nextToken(); // eat {

  auto expr = parser::parse(lex);
  if (!expr) return nullptr; // error forwarding

  lexer::Token endPosTok;
  if (!parser::match(lex, &endPosTok, lexer::tok_cbrace_template)) return nullptr;

  std::vector<std::unique_ptr<syntax::Expr>> params;
  while (expr->getType() == syntax::expr_biop
      && dynamic_cast<syntax::BiOpExpr&>(*expr).getOperator() == lexer::op_comma) {
    auto &biopexpr = dynamic_cast<syntax::BiOpExpr&>(*expr);
    // op_comma is left-associative binary operator
    params.insert(params.begin(), biopexpr.moveRHS());
    expr = biopexpr.moveLHS(); // iterator step
  }

  // last lhs to params
  params.insert(params.begin(), std::move(expr));

  return std::make_unique<syntax::TemplateExpr>(
      lexer::Position(startPos, endPosTok.getPosition()), std::move(params));
}

static std::unique_ptr<syntax::FuncParamExpr> _parsePrimaryFunctionParam(lexer::Lexer &lex) noexcept {
  auto param = parser::parse(lex, 0, true);
  if (!param) return nullptr;

  auto parampos = param->getPosition();
  std::string name;
  std::unique_ptr<syntax::Expr>  semanticType;
  std::unique_ptr<syntax::Expr> guard;
  std::unique_ptr<syntax::Expr> guardResult;

  if (param->getType() == syntax::expr_biop
      && dynamic_cast<syntax::BiOpExpr&>(*param).getOperator() == lexer::op_bor) {
    auto &biopexpr0 = dynamic_cast<syntax::BiOpExpr&>(*param);
    semanticType = biopexpr0.moveLHS();

    if (biopexpr0.getRHS().getType() == syntax::expr_biop
        && dynamic_cast<syntax::BiOpExpr&>(biopexpr0.getRHS()).getOperator() == lexer::op_impl) {
      auto &biopexpr1 = dynamic_cast<syntax::BiOpExpr&>(biopexpr0.getRHS());
      guard = biopexpr1.moveLHS();
      guardResult = biopexpr1.moveRHS();
    } else {
      guard = biopexpr0.moveRHS();
    }
  } else {
    semanticType = std::move(param);
  }

  if (semanticType->getType() == syntax::expr_biop
      && dynamic_cast<syntax::BiOpExpr&>(*semanticType).getOperator() == lexer::op_decl) {
    auto &biopexpr = dynamic_cast<syntax::BiOpExpr&>(*semanticType);
    if (biopexpr.getLHS().getType() != syntax::expr_id) {
      syntax::reportSyntaxError(lex,
          biopexpr.getLHS().getPosition(),
          "Expected identifier.");
      return nullptr;
    }

    name = dynamic_cast<syntax::IdExpr&>(biopexpr.getLHS()).getIdentifier();
    semanticType = biopexpr.moveRHS();
  } else {
    name = "_"; // == no name
  }

  return std::make_unique<syntax::FuncParamExpr>(
      parampos, name,
      std::move(semanticType), std::move(guard), std::move(guardResult));
}

static std::vector<std::unique_ptr<syntax::FuncParamExpr>> _parsePrimaryFunctionParams(lexer::Lexer &lex, bool &err) noexcept {
  err = false;
  lex.skipNewLine = true;

  std::vector<std::unique_ptr<syntax::FuncParamExpr>> result;

  lexer::Token startPosTok;
  if (!parser::match(lex, &startPosTok, lexer::tok_obrace)) { // eat (
      err = true;
      return result;
  }

  while (lex.currentToken() != lexer::tok_cbrace
      && lex.currentToken() != lexer::tok_eof) {

    if (result.size() > 0) {
      if (lex.currentToken() != lexer::op_comma) {
        err = true;
        syntax::reportSyntaxError(lex,
            lex.currentToken().getPosition(),
            "Expected tokens ',' or ')'.");
        return result;
      }

      lex.nextToken(); // eat ,
    }

    auto param = _parsePrimaryFunctionParam(lex);
    if (!param) {
      err = true;
      return result;
    }

    result.push_back(std::move(param));
  }

  lex.skipNewLine = false;

  if (!parser::match(lex, nullptr, lexer::tok_cbrace)) {
    err = true;
    return result;
  }

  return result;
}

static std::vector<std::string> _parsePrimaryFunctionName(lexer::Lexer &lex, bool &err) noexcept {
  err = false;

  std::vector<std::string> result;
  if (lex.currentToken() == lexer::tok_id) {
    result.push_back(lex.currentToken().getString());
    lex.nextToken(); // eat id
    while (lex.currentToken() == lexer::op_mem) {
      lex.nextToken(); // eat .

      lexer::Token tokid;
      if (!parser::match(lex, &tokid, lexer::tok_id)) {
        err = true;
        return result;
      }

      result.push_back(tokid.getString());
    }
  }

  return result;
}

static std::unique_ptr<syntax::FuncExpr> _parsePrimaryFunction(lexer::Lexer &lex) noexcept {
  lexer::Position pos = lex.currentToken().getPosition();
  bool virtualFunc = lex.currentToken() == lexer::tok_vfunc;
  lex.nextToken(); // eat 'func'/'Func'

  std::unique_ptr<syntax::TemplateExpr> templ = nullptr;
  if (lex.currentToken() == lexer::tok_obrace_template) {
    // Template
    templ = _parsePrimaryTemplate(lex);
    if (!templ) return nullptr;
  }

  bool funcNameErr;
  auto funcname = _parsePrimaryFunctionName(lex, funcNameErr);
  if (funcNameErr) return nullptr; // error forwarding

  std::vector<std::unique_ptr<syntax::FuncParamExpr>> params;
  if (lex.currentToken() == lexer::tok_obrace) {
    // Function parameters
    bool errorParams;
    params = _parsePrimaryFunctionParams(lex, errorParams);
    if (errorParams) return nullptr;
  }

  std::unique_ptr<syntax::Expr> resultType;
  if (lex.currentToken() == lexer::op_decl) {
    // Return type
    lex.nextToken(); // eat :

    resultType = parser::parse(lex);
    if (!resultType) return nullptr; // error forwarding
  }

  if (funcname.size() == 0) {
    if (virtualFunc) {
      syntax::reportSyntaxError(lex,
          pos, "Function types mustn't be virtual functions.");
      return nullptr;
    }

    if (templ) {
      syntax::reportSyntaxError(lex,
          templ->getPosition(), "Function types mustn't have a template.");
      return nullptr;
    }

    // Type
    return std::make_unique<syntax::FuncExpr>(
        pos, funcname,
        nullptr, std::move(resultType), std::move(params), nullptr, false);
  }

  if (lex.currentToken() == lexer::tok_delim) {
    lex.nextToken(); // eat ;

    if (funcname.size() != 1) {
      syntax::reportSyntaxError(lex, pos,
          "Reference name of declared function must be just 1 identifier.");
      return nullptr;
    }

    // Declared (but not defined function)
    return std::make_unique<syntax::FuncExpr>(
        pos, funcname,
        std::move(templ), std::move(resultType),
        std::move(params), nullptr, virtualFunc);
  }

  if (!parser::match(lex, nullptr, lexer::tok_eol)) return nullptr;

  // Defined
  auto program = parser::parseProgram(lex, false);
  if (!program) return nullptr;

  if (!parser::match(lex, nullptr, lexer::tok_delim)) return nullptr;

  return std::make_unique<syntax::FuncExpr>(
      pos, funcname,
      std::move(templ), std::move(resultType),
      std::move(params), std::move(program), virtualFunc);
}

static std::vector<std::unique_ptr<syntax::Expr>> _parseInherited(
    lexer::Lexer &lex, bool &error) noexcept {
  error = false;

  if (lex.currentToken() != lexer::op_tcast)
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
    result.insert(result.begin(), biopexpr.moveRHS());
    expr = biopexpr.moveLHS(); // iterator step
  }

  // The last expr is no biopexpr, so push to result
  result.insert(result.begin(), std::move(expr));

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
