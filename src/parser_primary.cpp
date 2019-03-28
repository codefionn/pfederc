#include "feder/parser.hpp"
using namespace feder;

static std::unique_ptr<syntax::IdExpr>
_parsePrimaryIdExpr(lexer::Tokenizer &lex) noexcept {
  lexer::Token tok = lex.currentToken();
  lex.nextToken(); // eat id

  const std::string &id = tok.getString();
  return std::make_unique<syntax::IdExpr>(tok.getPosition(), id);
}

static std::unique_ptr<syntax::NumExpr>
_parsePrimaryNumExpr(lexer::Tokenizer &lex) noexcept {
  lexer::Token tok = lex.currentToken();
  lex.nextToken(); // eat num

  lexer::NumberType numType = tok.getNumberType();
  lexer::NumberValue numVal = tok.getNumberValue();
  return std::make_unique<syntax::NumExpr>(tok.getPosition(), numType, numVal);
}

static std::unique_ptr<syntax::StrExpr>
_parsePrimaryString(lexer::Tokenizer &lex) noexcept {
  lexer::Token tok = lex.currentToken();
  lex.nextToken(); // eat str

  return std::make_unique<syntax::StrExpr>(tok.getPosition(), tok.getString());
}

static std::unique_ptr<syntax::CharExpr>
_parsePrimaryChar(lexer::Tokenizer &lex) noexcept {
  lexer::Token tok = lex.currentToken();
  lex.nextToken(); // eat str

  return std::make_unique<syntax::CharExpr>(tok.getPosition(),
                                            tok.getCharacter());
}

static std::unique_ptr<syntax::BraceExpr>
_parsePrimaryBraceExpr(lexer::Tokenizer &lex) noexcept {
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

  bool err = false;

  std::unique_ptr<syntax::Expr> expr(parser::parse(lex));
  if (!expr)
    err = true;

  lex.skipNewLine = false;

  lexer::Token tokcbrace;
  if (!parser::match(lex, &tokcbrace, lexer::tok_cbrace))
    err = true;

  if (err)
    return nullptr;

  lexer::Position pos(posStart, tokcbrace.getPosition());
  return std::make_unique<syntax::BraceExpr>(pos, std::move(expr));
}

static std::unique_ptr<syntax::UnOpExpr>
_parsePrimaryUnOpExpr(lexer::Tokenizer &lex) noexcept {
  if (!lexer::isValidOperatorPosition(lex.currentToken().getOperator(),
                                      lexer::op_lunary)) {
    syntax::reportSyntaxError(
        lex, lex.currentToken().getPosition(),
        "Expected primary token or left-side unary operator.");
    return nullptr;
  }

  lexer::Token opTok = lex.currentToken();
  lex.nextToken(); // eat op
  std::unique_ptr<syntax::Expr> expr(
      parser::parse(lex, opTok.getPrecedence(lexer::op_lunary)));
  if (!expr)
    return nullptr;

  return std::make_unique<syntax::UnOpExpr>(
      lexer::Position(opTok.getPosition(), expr->getPosition()),
      lexer::op_lunary, opTok.getOperator(), std::move(expr));
}

static std::unique_ptr<syntax::Expr>
_parsePrimaryArray(lexer::Tokenizer &lex) noexcept {
  lex.skipNewLine = true;

  lexer::Position posStart = lex.currentToken().getPosition();
  lex.nextToken(); // eat [

  bool err = false;

  auto expr = parser::parse(lex);
  if (!expr)
    err = true; // error forwarding

  if (lex.currentToken() == lexer::tok_delim) {
    // Constructor
    lex.nextToken(); // eat ;

    auto expr1 = parser::parse(lex);
    if (!expr1)
      err = true; // error forwarding

    lex.skipNewLine = false;

    lexer::Token posEndTok;
    if (!parser::match(lex, &posEndTok, lexer::tok_cbrace_array))
      err = true;

    if (err)
      return nullptr;

    return std::make_unique<syntax::ArrayConExpr>(
        lexer::Position(posStart, posEndTok.getPosition()), std::move(expr),
        std::move(expr1));
  }

  lex.skipNewLine = false;

  lexer::Token posEndTok;
  if (!parser::match(lex, &posEndTok, lexer::tok_cbrace_array))
    return nullptr;

  if (expr->getType() == syntax::expr_biop &&
      dynamic_cast<syntax::BiOpExpr &>(*expr).getOperator() ==
          lexer::op_comma) {
    // List
    std::vector<std::unique_ptr<syntax::Expr>> objs;
    // Iterator through LHSs of expr and append RHS to objs
    while (expr->getType() == syntax::expr_biop &&
           dynamic_cast<syntax::BiOpExpr &>(*expr).getOperator() ==
               lexer::op_comma) {
      auto &biopexpr = dynamic_cast<syntax::BiOpExpr &>(*expr);

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

static std::unique_ptr<syntax::TemplateExpr>
_parsePrimaryTemplate(lexer::Tokenizer &lex) noexcept {
  lex.skipNewLine = true;

  lexer::Position startPos = lex.currentToken().getPosition();
  lex.nextToken(); // eat {

  bool err = false;

  auto expr = parser::parse(lex);
  if (!expr)
    err = true; // error forwarding

  lex.skipNewLine = false;

  lexer::Token endPosTok;
  if (!parser::match(lex, &endPosTok, lexer::tok_cbrace_template))
    err = true;

  if (err)
    return nullptr;

  std::vector<std::unique_ptr<syntax::Expr>> params;
  while (expr->getType() == syntax::expr_biop &&
         dynamic_cast<syntax::BiOpExpr &>(*expr).getOperator() ==
             lexer::op_comma) {
    auto &biopexpr = dynamic_cast<syntax::BiOpExpr &>(*expr);
    // op_comma is left-associative binary operator
    params.insert(params.begin(), biopexpr.moveRHS());
    expr = biopexpr.moveLHS(); // iterator step
  }

  // last lhs to params
  params.insert(params.begin(), std::move(expr));

  return std::make_unique<syntax::TemplateExpr>(
      lexer::Position(startPos, endPosTok.getPosition()), std::move(params));
}

static std::unique_ptr<syntax::FuncParamExpr>
_parsePrimaryFunctionParam(lexer::Tokenizer &lex) noexcept {
  auto param = parser::parse(lex, 0, true);
  if (!param)
    return nullptr;

  auto parampos = param->getPosition();
  std::string name;
  std::unique_ptr<syntax::Expr> semanticType;
  std::unique_ptr<syntax::Expr> guard;
  std::unique_ptr<syntax::Expr> guardResult;

  if (param->getType() == syntax::expr_biop &&
      dynamic_cast<syntax::BiOpExpr &>(*param).getOperator() == lexer::op_bor) {
    auto &biopexpr0 = dynamic_cast<syntax::BiOpExpr &>(*param);
    semanticType = biopexpr0.moveLHS();

    if (biopexpr0.getRHS().getType() == syntax::expr_biop &&
        dynamic_cast<syntax::BiOpExpr &>(biopexpr0.getRHS()).getOperator() ==
            lexer::op_impl) {
      auto &biopexpr1 = dynamic_cast<syntax::BiOpExpr &>(biopexpr0.getRHS());
      guard = biopexpr1.moveLHS();
      guardResult = biopexpr1.moveRHS();
    } else {
      guard = biopexpr0.moveRHS();
    }
  } else {
    semanticType = std::move(param);
  }

  if (semanticType->getType() == syntax::expr_biop &&
      dynamic_cast<syntax::BiOpExpr &>(*semanticType).getOperator() ==
          lexer::op_decl) {
    auto &biopexpr = dynamic_cast<syntax::BiOpExpr &>(*semanticType);
    if (biopexpr.getLHS().getType() != syntax::expr_id) {
      syntax::reportSyntaxError(lex, biopexpr.getLHS().getPosition(),
                                "Expected identifier.");
      return nullptr;
    }

    name = dynamic_cast<syntax::IdExpr &>(biopexpr.getLHS()).getIdentifier();
    semanticType = biopexpr.moveRHS();
  } else {
    name = "_"; // == no name
  }

  return std::make_unique<syntax::FuncParamExpr>(
      parampos, name, std::move(semanticType), std::move(guard),
      std::move(guardResult));
}

static std::vector<std::unique_ptr<syntax::FuncParamExpr>>
_parsePrimaryFunctionParams(lexer::Tokenizer &lex, bool &err) noexcept {
  err = false;
  lex.skipNewLine = true;

  std::vector<std::unique_ptr<syntax::FuncParamExpr>> result;

  lexer::Token startPosTok;
  if (!parser::match(lex, &startPosTok, lexer::tok_obrace)) { // eat (
    err = true;
    return result;
  }

  while (lex.currentToken() != lexer::tok_cbrace &&
         lex.currentToken() != lexer::tok_eof) {
    if (!lex.advanced()) {
      err = true;
      break;
    }

    if (result.size() > 0) {
      if (lex.currentToken() != lexer::op_comma) {
        err = true;
        syntax::reportSyntaxError(lex, lex.currentToken().getPosition(),
                                  "Expected tokens ',' or ')'.");
        continue;
      }

      lex.nextToken(); // eat ,
    }

    auto param = _parsePrimaryFunctionParam(lex);
    if (!param)
      err = true;
    else
      result.push_back(std::move(param));
  }

  lex.skipNewLine = false;

  if (!parser::match(lex, nullptr, lexer::tok_cbrace)) {
    err = true;
    return result;
  }

  return result;
}

static std::vector<std::string> _parsePrimaryFunctionName(lexer::Tokenizer &lex,
                                                          bool &err) noexcept {
  err = false;

  std::vector<std::string> result;
  if (lex.currentToken() == lexer::tok_id) {
    result = parser::parseIdentifierCall(lex);
    if (result.empty())
      err = true;
  }

  return result;
}

static std::unique_ptr<syntax::FuncExpr>
_parsePrimaryFunction(lexer::Tokenizer &lex) noexcept {
  lexer::Position pos = lex.currentToken().getPosition();
  bool virtualFunc = lex.currentToken() == lexer::tok_vfunc;
  lex.nextToken(); // eat 'func'/'Func'

  lex.pushSkipNewLine();

  std::unique_ptr<syntax::TemplateExpr> templ = nullptr;
  if (lex.currentToken() == lexer::tok_obrace_template) {
    // Template
    templ = _parsePrimaryTemplate(lex);
    if (!templ)
      return nullptr;
  }

  bool err = false;

  bool funcNameErr = false;
  auto funcname = _parsePrimaryFunctionName(lex, funcNameErr);
  if (funcNameErr)
    err = true; // error forwarding

  std::vector<std::unique_ptr<syntax::FuncParamExpr>> params;
  if (lex.currentToken() == lexer::tok_obrace) {
    // Function parameters
    bool errorParams;
    params = _parsePrimaryFunctionParams(lex, errorParams);
    if (errorParams)
      err = true;
  }

  std::unique_ptr<syntax::Expr> resultType;
  if (lex.currentToken() == lexer::op_decl) {
    // Return type
    lex.nextToken(); // eat :

    resultType = parser::parse(lex);
    if (!resultType)
      err = true; // error forwarding
  }

  if (funcname.size() == 0) {
    // No function name => It's a function type
    if (virtualFunc) {
      syntax::reportSyntaxError(lex, pos,
                                "Function types mustn't be virtual functions.");
      err = true;
    }

    if (templ) {
      syntax::reportSyntaxError(lex, templ->getPosition(),
                                "Function types mustn't have a template.");
      err = true;
    }

    lex.popSkipNewLine();

    if (err)
      return nullptr;

    // Type
    return std::make_unique<syntax::FuncExpr>(
        pos, funcname, nullptr, std::move(resultType), std::move(params),
        nullptr, false);
  }

  if (lex.currentToken() == lexer::tok_delim) {
    // Declared (but not defined function)
    lex.nextToken(); // eat ;

    if (funcname.size() != 1) {
      syntax::reportSyntaxError(
          lex, pos,
          "Reference name of declared function must be just 1 identifier.");
      err = true;
    }

    lex.popSkipNewLine();

    if (err)
      return nullptr;

    return std::make_unique<syntax::FuncExpr>(
        pos, funcname, std::move(templ), std::move(resultType),
        std::move(params), nullptr, virtualFunc);
  }

  if (!parser::match(lex, nullptr, lexer::tok_eol))
    err = true;

  // Defined functions
  auto program = parser::parseProgram(lex, false);
  if (!program)
    err = true;

  if (!parser::match(lex, nullptr, lexer::tok_delim))
    err = true;

  lex.popSkipNewLine();

  if (err)
    return nullptr;

  return std::make_unique<syntax::FuncExpr>(
      pos, funcname, std::move(templ), std::move(resultType), std::move(params),
      std::move(program), virtualFunc);
}

static std::vector<std::unique_ptr<syntax::Expr>>
_parseInherited(lexer::Tokenizer &lex, bool &error) noexcept {
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

  while (expr->getType() == syntax::expr_biop &&
         dynamic_cast<syntax::BiOpExpr &>(*expr).getOperator() ==
             lexer::op_comma) {
    auto &biopexpr = dynamic_cast<syntax::BiOpExpr &>(*expr);

    // , is left associative
    result.insert(result.begin(), biopexpr.moveRHS());
    expr = biopexpr.moveLHS(); // iterator step
  }

  // The last expr is no biopexpr, so push to result
  result.insert(result.begin(), std::move(expr));

  return result;
}

static void _parsePrimaryClassBody(
    lexer::Tokenizer &lex, bool &err, const std::string &className,
    std::vector<std::unique_ptr<syntax::Expr>> &attributes,
    std::vector<std::unique_ptr<syntax::FuncExpr>> &constructors,
    std::vector<std::unique_ptr<syntax::FuncExpr>> &functions) noexcept {
  while (lex.currentToken() != lexer::tok_eof &&
         lex.currentToken() != lexer::tok_delim) {
    if (!lex.advanced()) {
      err = true;
      break;
    }

    if (lex.currentToken() == lexer::tok_eol) {
      lex.nextToken(); // eat newline
      continue;
    }

    auto expr = parser::parse(lex);
    if (!expr) {
      err = true;
      continue;
    }

    switch (expr->getType()) {
    case syntax::expr_func: {
      if (dynamic_cast<syntax::FuncExpr &>(*expr).isType()) {
        syntax::reportSyntaxError(
            lex, expr->getPosition(),
            "Excpected function or variable declaration.");
        err = true;
        break;
      }

      auto &funcexpr = dynamic_cast<syntax::FuncExpr &>(*expr);
      if (funcexpr.getName().size() > 1) {
        syntax::reportSyntaxError(
            lex, expr->getPosition(),
            "Function was expected to have just one identifier.");
        err = true;
        break;
      }

      const std::string &funcname = funcexpr.getName()[0];
      if (funcname == className || funcname == "_" + className) {
        if (funcexpr.isVirtual()) {
          syntax::reportSyntaxError(
              lex, expr->getPosition(),
              "Constructors must not be virtual functions!");
          err = true;
          break;
        }

        // Constructor
        constructors.push_back(std::move(
            reinterpret_cast<std::unique_ptr<syntax::FuncExpr> &>(expr)));
        break;
      }

      // Normal function
      functions.push_back(std::move(
          reinterpret_cast<std::unique_ptr<syntax::FuncExpr> &>(expr)));
      break;
    }
    case syntax::expr_biop:
      if (dynamic_cast<syntax::BiOpExpr &>(*expr).getOperator() ==
          lexer::op_decl) {
        attributes.push_back(std::move(expr));
        break;
      }
    default:
      syntax::reportSyntaxError(lex, expr->getPosition(),
                                "Excpected function or variable declaration.");
      err = true;
      break;
    }

    if (!parser::match(lex, nullptr, lexer::tok_eol))
      err = true;
  }
}

static std::unique_ptr<syntax::ClassExpr>
_parsePrimaryClass(lexer::Tokenizer &lex) noexcept {
  lex.pushSkipNewLine();

  lexer::Position startPos = lex.currentToken().getPosition();
  lex.nextToken(); // eat 'class'

  bool err = false;

  std::unique_ptr<syntax::TemplateExpr> templ;
  if (lex.currentToken() == lexer::tok_obrace_template) {
    templ = _parsePrimaryTemplate(lex);
    if (!templ)
      err = true; // error forwarding
  }

  lexer::Token idTok;
  if (!parser::match(lex, &idTok, lexer::tok_id))
    err = true;

  // deps
  bool errorInherited;
  auto traits = _parseInherited(lex, errorInherited);
  if (errorInherited)
    err = true;

  if (!parser::match(lex, nullptr, lexer::tok_eol))
    err = true;

  std::vector<std::unique_ptr<syntax::Expr>> attributes;
  std::vector<std::unique_ptr<syntax::FuncExpr>> constructors;
  std::vector<std::unique_ptr<syntax::FuncExpr>> functions;

  // Parse body of class
  _parsePrimaryClassBody(lex, err, idTok.getString(),
                         attributes, constructors, functions);

  if (!parser::match(lex, nullptr, lexer::tok_delim))
    err = true;

  lex.popSkipNewLine();

  if (err)
    return nullptr;

  return std::make_unique<syntax::ClassExpr>(
      lexer::Position(startPos, idTok.getPosition()), idTok.getString(),
      std::move(templ), std::move(traits), std::move(attributes),
      std::move(constructors), std::move(functions));
}

static std::unique_ptr<syntax::TraitExpr>
_parsePrimaryTrait(lexer::Tokenizer &lex) noexcept {
  lex.pushSkipNewLine();

  lexer::Position pos = lex.currentToken().getPosition();
  lex.nextToken(); // eat 'trait'

  bool err = false;

  std::unique_ptr<syntax::TemplateExpr> templ;
  if (lex.currentToken() == lexer::tok_obrace_template) {
    templ = _parsePrimaryTemplate(lex);
    if (!templ)
      err = true; // error forwarding
  }

  lexer::Token idTok;
  if (!parser::match(lex, &idTok, lexer::tok_id))
    err = true;

  // deps
  bool errorInherited;
  auto traits = _parseInherited(lex, errorInherited);
  if (errorInherited)
    err = true;

  if (!parser::match(lex, nullptr, lexer::tok_eol))
    err = true;

  std::vector<std::unique_ptr<syntax::FuncExpr>> funs;

  while (lex.currentToken() == lexer::tok_vfunc ||
         lex.currentToken() == lexer::tok_eol) {
    if (!lex.advanced()) {
      err = true;
      break;
    }

    if (lex.currentToken() == lexer::tok_eol) {
      lex.nextToken(); // eat newline
      continue;
    }

    auto fun = _parsePrimaryFunction(lex);
    if (!fun) {
      err = true; // error forwarding

      if (!parser::match(lex, nullptr, lexer::tok_eol))
        err = true;
      
      continue;
    }

    if (!fun->isDeclared() || !fun->isVirtual()) {
      syntax::reportSyntaxError(
          lex, fun->getPosition(),
          "Traits must have just declared, virtual functions.");
      err = true;
    }

    funs.push_back(std::move(fun));

    if (!parser::match(lex, nullptr, lexer::tok_eol))
      err = true;
  }

  if (!parser::match(lex, nullptr, lexer::tok_delim))
    err = true;

  if (traits.empty() && funs.empty()) {
    lex.reportSemanticError(
        "Traits with no inherited traits must declare at least one function.",
        lexer::Position(pos, idTok.getPosition()));
    err = true;
  }

  lex.popSkipNewLine();

  if (err)
    return nullptr;

  return std::make_unique<syntax::TraitExpr>(
      lexer::Position(pos, idTok.getPosition()), idTok.getString(),
      std::move(templ), std::move(traits), std::move(funs));
}

static std::unique_ptr<syntax::EnumExpr>
_parsePrimaryEnum(lexer::Tokenizer &lex) noexcept {
  lex.pushSkipNewLine();

  lexer::Position startPos = lex.currentToken().getPosition();
  lex.nextToken(); // eat 'enum'

  bool err = false;

  std::unique_ptr<syntax::TemplateExpr> templ;
  if (lex.currentToken() == lexer::tok_obrace_template) {
    templ = _parsePrimaryTemplate(lex);
    if (!templ)
      err = true; // error forwarding
  }

  lexer::Token tokId;
  if (!parser::match(lex, &tokId, lexer::tok_id))
    err = true;

  if (!parser::match(lex, nullptr, lexer::tok_eol))
    err = true;

  std::vector<std::unique_ptr<syntax::Expr>> constructors;
  while (lex.currentToken() != lexer::tok_delim
      && lex.currentToken() != lexer::tok_eof) {
    if (!lex.advanced()) {
      err = true;
      break;
    }

    if (lex.currentToken() == lexer::tok_eol) {
      lex.nextToken(); // eat newline
      continue;
    }

    auto expr = parser::parse(lex, 0, true);
    if (!expr) {
      err = true;
      continue;
    }

    if (expr->getType() == syntax::expr_id) {
      constructors.push_back(std::move(expr));
    } else if (expr->getType() == syntax::expr_biop) {
      if (dynamic_cast<syntax::BiOpExpr&>(*expr).getOperator() != lexer::op_fncall) {
        lex.reportSyntaxError(
            "Expected either identifier or constructor.",
            expr->getPosition());
        err = true;
        continue;
      }

      if (dynamic_cast<syntax::BiOpExpr&>(*expr).getLHS().getType() != syntax::expr_id) {
        auto &biopexpr = dynamic_cast<syntax::BiOpExpr&>(*expr);
        lex.reportSyntaxError(
            "Expected identifier.",
            biopexpr.getLHS().getPosition());
        err = true;
        continue;
      }

      constructors.push_back(std::move(expr));
    } else {
      lex.reportSyntaxError(
          "Expected either identifier or constructor.",
          expr->getPosition());
      err = true;
      continue;
    }

    if (!parser::match(lex, nullptr, lexer::tok_eol)) {
      err = true;
      continue;
    }
  }

  if (!parser::match(lex, nullptr, lexer::tok_delim))
    err = true;

  lex.popSkipNewLine();

  if (err)
    return nullptr;

  return std::make_unique<syntax::EnumExpr>(
      lexer::Position(startPos, tokId.getPosition()),
      tokId.getString(), std::move(templ), std::move(constructors));
}

static std::unique_ptr<syntax::NmspExpr>
_parsePrimaryNamespace(lexer::Tokenizer &lex) noexcept {
  lex.pushSkipNewLine();

  lexer::Position pos = lex.currentToken().getPosition();
  lex.nextToken(); // eat 'namespace'

  bool err = false;

  lexer::Token tokId;
  if (!parser::match(lex, &tokId, lexer::tok_id))
    err = true;

  if (!parser::match(lex, nullptr, lexer::tok_eol))
    err = true;

  auto program = parser::parseProgram(lex, false);
  if (!program || program->hasError())
    err = true;

  if (!parser::match(lex, nullptr, lexer::tok_delim))
    err = true;

  lex.popSkipNewLine();

  if (err)
    return nullptr;

  return std::make_unique<syntax::NmspExpr>(
      lexer::Position(pos, tokId.getPosition()), tokId.getString(),
      std::move(program));
}

static syntax::IfCaseExpr
_parsePrimaryIfCase(lexer::Tokenizer &lex) noexcept {
  if (!parser::match(lex, nullptr, lexer::tok_if))
    return syntax::IfCaseExpr(nullptr, nullptr);

  bool err = false;

  auto condition = parser::parse(lex);
  if (!condition)
    err = true;

  if (!parser::match(lex, nullptr, lexer::tok_eol))
    err = true;

  auto program = parser::parseProgram(lex, false);
  if (!program || program->hasError())
    err = true;

  if (lex.currentToken() != lexer::tok_delim
      && lex.currentToken() != lexer::tok_else) {
    syntax::reportSyntaxError(lex,
        lex.currentToken().getPosition(),
        "Expected token ';' or 'else'.");
    err = true;
  }

  if (err)
    return syntax::IfCaseExpr(nullptr, nullptr);

  return syntax::IfCaseExpr(std::move(condition), std::move(program));
}

static std::unique_ptr<syntax::IfExpr>
_parsePrimaryIf(lexer::Tokenizer &lex) noexcept {
  lex.pushSkipNewLine();

  lexer::Position startPos = lex.currentToken().getPosition();

  std::vector<syntax::IfCaseExpr> ifCases;
  std::unique_ptr<syntax::Program> elseCase;

  bool err = false;
  bool isfirst = true;

  while (lex.currentToken() != lexer::tok_delim
      && lex.currentToken() != lexer::tok_eof) {
    if (!lex.advanced()) {
      err = true;
      break;
    }

    if (lex.currentToken() == lexer::tok_eol) {
      lex.nextToken(); // eat newline
      continue;
    }

    if (!isfirst && !parser::match(lex, nullptr, lexer::tok_else))
      return nullptr;

    if (lex.currentToken() == lexer::tok_if) {
      isfirst = false;

      syntax::IfCaseExpr ifcase(_parsePrimaryIfCase(lex));
      if (!ifcase.first) {
        err = true;
        continue;
      }

      ifCases.push_back(std::move(ifcase));
    } else {
      if (!parser::match(lex, nullptr, lexer::tok_eol));

      auto program = parser::parseProgram(lex, false);
      if (!program || program->hasError())
        err = true;
      else
        elseCase = std::move(program);

      break;
    }
  }

  if (!parser::match(lex, nullptr, lexer::tok_delim))
    err = true;

  lex.popSkipNewLine();

  if (err)
    return nullptr;

  return std::make_unique<syntax::IfExpr>(
      lexer::Position(startPos, ifCases[0].first->getPosition()),
      std::move(ifCases), std::move(elseCase));
}

syntax::MatchCaseExpr
_parsePrimaryMatchCase(lexer::Tokenizer &lex) noexcept {
  bool err = false;

  // Parse identifiercall
  auto idExpr = parser::parseIdentifierCallExpr(lex);
  if (!idExpr)
    err = true;

  if (lex.currentToken() == lexer::op_impl) {
    lex.nextToken();

    // Match case body   
    auto program = parser::parseProgram(lex, false);
    if (!program || program->hasError())
      err = true;

    if (!parser::match(lex, nullptr, lexer::tok_delim))
      err = true;

    if (err)
      return syntax::MatchCaseExpr(nullptr, nullptr);

    return syntax::MatchCaseExpr(std::move(idExpr), std::move(program));
  } else if (lex.currentToken() != lexer::tok_obrace) {
    syntax::reportSyntaxError(lex,
        lex.currentToken().getPosition(),
        "Expected token '(' or '=>'.");
    return syntax::MatchCaseExpr(nullptr, nullptr);
  }

  auto constructor = parser::parsePrimary(lex);
  if (!constructor)
    err = true;

  // TODO: Check if valid constructor

  idExpr = std::make_unique<syntax::BiOpExpr>(
      lexer::Position(idExpr->getPosition(), constructor->getPosition()),
      lexer::op_fncall, std::move(idExpr), std::move(constructor));

  if (!parser::match(lex, nullptr, lexer::tok_op, lexer::op_impl))
    return syntax::MatchCaseExpr(nullptr, nullptr);

  // Match case body
  auto program = parser::parseProgram(lex, false);
  if (!program || program->hasError())
    err = true;

  if (!parser::match(lex, nullptr, lexer::tok_delim))
    err = true;

  if (!(idExpr->getType() == syntax::expr_id
        && dynamic_cast<syntax::IdExpr&>(*idExpr).getIdentifier() == "_")
      && !idExpr->hasReturn()) {
    lex.reportSemanticError(
        "Expected default case or secondary expression.",
        idExpr->getPosition());
    err = true;
  }

  if (err)
    return syntax::MatchCaseExpr(nullptr, nullptr);

  return syntax::MatchCaseExpr(std::move(idExpr), std::move(program));
}

std::unique_ptr<syntax::Expr>
_parsePrimaryMatch(lexer::Tokenizer &lex) noexcept {
  lex.pushSkipNewLine();

  lexer::Position startPos(lex.currentToken().getPosition());
  lex.nextToken(); // eat match

  bool err = false;

  auto enumValExpr = parser::parse(lex);
  if (!enumValExpr)
    err = true; // error forwarding

  if (!parser::match(lex, nullptr, lexer::tok_eol))
    err = true;

  std::unique_ptr<syntax::Program> defaultCase(nullptr);
  std::vector<syntax::MatchCaseExpr> matchCases;
  while (lex.currentToken() != lexer::tok_delim
      && lex.currentToken() != lexer::tok_eof) {
    if (!lex.advanced()) {
      err = true;
      break;
    }

    if (lex.currentToken() == lexer::tok_eol) {
      lex.nextToken(); // eat newline
      continue;
    }

    auto matchcase = _parsePrimaryMatchCase(lex);
    if (!matchcase.first)
      err = true;
    else {
      if (matchcase.first->getType() == syntax::expr_id
          && dynamic_cast<syntax::IdExpr&>(*matchcase.first).getIdentifier() == "_") {
        if (defaultCase) {
          lex.reportSemanticError(
              "Match statement must have just one default case.",
              matchcase.first->getPosition());
        } else 
          defaultCase = std::move(matchcase.second);
      } else {
        matchCases.push_back(std::move(matchcase));
      }
    }

    if (!parser::match(lex, nullptr, lexer::tok_eol))
      err = true;
  }

  if (!parser::match(lex, nullptr, lexer::tok_delim))
    err = true;

  if (matchCases.empty()) {
    err = true;
    lex.reportSemanticError("Expected at least one match case.",
        lexer::Position(startPos, enumValExpr->getPosition()));
  }

  lex.popSkipNewLine();

  if (err)
    return nullptr;

  return std::make_unique<syntax::MatchExpr>(
      lexer::Position(startPos, enumValExpr->getPosition()),
      std::move(enumValExpr), std::move(matchCases),
      std::move(defaultCase));
}

static std::unique_ptr<syntax::ForExpr>
_parsePrimaryFor(lexer::Tokenizer &lex) noexcept {
  lex.pushSkipNewLine();

  lexer::Position startPos(lex.currentToken().getPosition());
  bool postCondition = lex.currentToken() == lexer::tok_do;
  lex.nextToken(); // eat for/do

  bool err = false;

  std::unique_ptr<syntax::Expr> init;
  std::unique_ptr<syntax::Expr> cond;
  std::unique_ptr<syntax::Expr> step;

  std::unique_ptr<syntax::Program> prog;

  if (postCondition && lex.currentToken() != lexer::tok_eol) {
    // optional init + optional step
    init = parser::parse(lex);
    if (!init)
      err = true; // error forwarding

    if (lex.currentToken() == lexer::tok_delim) {
      lex.nextToken(); // eat ;
      step = parser::parse(lex);
      if (!step)
        err = true; // error forwarding
    } else {
      step = std::move(init);
      init = nullptr;
    }
  } else if (!postCondition) {
    // optional init + condition + optional step
    init = parser::parse(lex);
    if (!init)
      err = true; // error forwarding

    if (lex.currentToken() == lexer::tok_delim) {
      lex.nextToken(); // eat ;

      cond = parser::parse(lex);
      if (!cond)
        err = true; // error forwarding

      if (lex.currentToken() == lexer::tok_delim) {
        lex.nextToken(); // eat ;

        step = parser::parse(lex);
        if (!step)
          err = true; // error forwarding
      } else {
        step = std::move(cond);
        cond = std::move(init);
        init = nullptr;
      }
    } else {
      cond = std::move(init);
      init = nullptr;
    }
  }

  if (!parser::match(lex, nullptr, lexer::tok_eol))
    err = true;

  prog = parser::parseProgram(lex, false);
  if (!prog)
    err = true; // error forwarding

  if (!parser::match(lex, nullptr, lexer::tok_delim))
    err = true;

  if (postCondition) {
    if (!parser::match(lex, nullptr, lexer::tok_for))
      err = true;
    else {
      cond = parser::parse(lex);
      if (!cond)
        err = true; // error forwarding
  
      if (!parser::match(lex, nullptr,
          std::vector<lexer::TokenType>{lexer::tok_eol, lexer::tok_eof}))
        err = true;
    }
  }

  lex.popSkipNewLine();

  if (err)
    return nullptr;

  return std::make_unique<syntax::ForExpr>(startPos,
      std::move(init), std::move(cond), std::move(step),
      std::move(prog), postCondition);
}

std::unique_ptr<syntax::Expr> parser::parsePrimary(lexer::Tokenizer &lex) noexcept {
  switch (lex.currentToken().getType()) {
  case lexer::tok_id:
    return _parsePrimaryIdExpr(lex);
  case lexer::tok_num:
    return _parsePrimaryNumExpr(lex);
  case lexer::tok_str:
    return _parsePrimaryString(lex);
  case lexer::tok_char:
    return _parsePrimaryChar(lex);
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
  case lexer::tok_enum:
    return _parsePrimaryEnum(lex);
  case lexer::tok_nmsp:
    return _parsePrimaryNamespace(lex);

  case lexer::tok_if:
    return _parsePrimaryIf(lex);
  case lexer::tok_match:
    return _parsePrimaryMatch(lex);
  case lexer::tok_for:
  case lexer::tok_do:
    return _parsePrimaryFor(lex);

  case lexer::tok_op:
    return _parsePrimaryUnOpExpr(lex);

  case lexer::tok_err:
    return nullptr;

  default:
    return syntax::reportSyntaxError(
        lex, lex.currentToken().getPosition(),
        "Not a primary token " + std::to_string(lex.currentToken().getType()) +
            ".");
  }
}
