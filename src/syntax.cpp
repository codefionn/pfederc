#include "feder/syntax.hpp"
using namespace feder::syntax;

Program::Program(std::vector<std::unique_ptr<Expr>> &&lines,
                 std::unique_ptr<Expr> &&returnExpr,
                 bool error) noexcept
    : lines(std::move(lines)), returnExpr(std::move(returnExpr)),
      error{error} {}

Program::~Program() {}

Expr::Expr(ExprType type, const feder::lexer::Position &pos) noexcept
    : type{type}, pos(pos) {}

Expr::~Expr() {}

IdExpr::IdExpr(ExprType type, const feder::lexer::Position &pos,
               const std::string &id) noexcept
    : Expr(type, pos), id(id) {
#ifdef SANITY
  FEDER_SANITY_CHECK(this->id.size() > 0, "id,size() == 0");
#endif
}

IdExpr::IdExpr(const feder::lexer::Position &pos,
               const std::string &id) noexcept
    : Expr(expr_id, pos), id(id) {
#ifdef SANITY
  FEDER_SANITY_CHECK(this->id.size() > 0, "id,size() == 0");
#endif
}

IdExpr::~IdExpr() {}

NumExpr::NumExpr(const feder::lexer::Position &pos,
                 feder::lexer::NumberType numType,
                 feder::lexer::NumberValue numVal) noexcept
    : Expr(expr_num, pos), numType{numType}, numVal{numVal} {}

NumExpr::~NumExpr() {}

StrExpr::StrExpr(const feder::lexer::Position &pos,
                 const std::string &str) noexcept
    : Expr(expr_str, pos), str(str) {}

StrExpr::~StrExpr() {}

CharExpr::CharExpr(const feder::lexer::Position &pos, char c) noexcept
    : Expr(expr_char, pos), c{c} {}

CharExpr::~CharExpr() {}

FuncParamExpr::FuncParamExpr(const feder::lexer::Position &pos,
                             const std::string &name,
                             std::unique_ptr<Expr> &&semanticType,
                             std::unique_ptr<Expr> &&guard,
                             std::unique_ptr<Expr> &&guardResult) noexcept
    : IdExpr(expr_func_param, pos, name), semanticType(std::move(semanticType)),
      guard(std::move(guard)), guardResult(std::move(guardResult)) {
#ifdef SANITY
  FEDER_SANITY_CHECK((bool) this->semanticType, "!semanticType");
#endif
}

FuncParamExpr::~FuncParamExpr() {}

FuncExpr::FuncExpr(const feder::lexer::Position &pos,
                   const std::vector<std::string> &name,
				   std::unique_ptr<CapsExpr> &&caps,
                   std::unique_ptr<TemplateExpr> &&templ,
                   std::unique_ptr<Expr> &&returnType,
                   std::vector<std::unique_ptr<FuncParamExpr>> &&params,
                   std::unique_ptr<Program> &&program,
                   bool virtualFunc) noexcept
    : Expr(expr_func, pos), name(name), caps(std::move(caps)),
	  templ(std::move(templ)),
      returnType(std::move(returnType)), params(std::move(params)),
      program(std::move(program)), virtualFunc{virtualFunc} {
#ifdef SANITY
  FEDER_SANITY_CHECK(!this->name.empty() || (!this->templ && !this->program), "Invalid Function Type");
  FEDER_SANITY_CHECK(this->name.empty() || ((bool) this->program || this->name.size() == 1), "Function decl. name.size() != 1");
#endif
}

FuncExpr::~FuncExpr() {}

ClassExpr::ClassExpr(
    const feder::lexer::Position &pos, const std::string &name,
    std::unique_ptr<TemplateExpr> &&templ,
    std::vector<std::unique_ptr<Expr>> &&traits,
    std::vector<std::unique_ptr<Expr>> &&attributes,
    std::vector<std::unique_ptr<FuncExpr>> &&constructors,
    std::vector<std::unique_ptr<FuncExpr>> &&functions) noexcept
    : IdExpr(expr_class, pos, name), templ(std::move(templ)),
      traits(std::move(traits)), attributes(std::move(attributes)),
      constructors(std::move(constructors)), functions(std::move(functions)) {}

ClassExpr::~ClassExpr() {}

EnumExpr::EnumExpr(const feder::lexer::Position &pos, const std::string &name,
                   std::unique_ptr<TemplateExpr> &&templ,
                   std::vector<std::unique_ptr<Expr>> &&constructors) noexcept
    : IdExpr(expr_enum, pos, name), templ(std::move(templ)),
      constructors(std::move(constructors)) {
#ifdef SANITY
  FEDER_SANITY_CHECK(this->constructors.size() > 0, "constructors.size() == 0");
#endif
}

EnumExpr::~EnumExpr() {}

TraitExpr::TraitExpr(
    const feder::lexer::Position &pos, const std::string &name,
    std::unique_ptr<TemplateExpr> &&templ,
    std::vector<std::unique_ptr<Expr>> &&traits,
    std::vector<std::unique_ptr<FuncExpr>> &&functions) noexcept
    : IdExpr(expr_trait, pos, name), templ(std::move(templ)),
      traits(std::move(traits)), functions(std::move(functions)) {
#ifdef SANITY
  FEDER_SANITY_CHECK(this->traits.size() > 0 || this->functions.size() > 0,
      "Either implement traits or declare functions.");
#endif
}

TraitExpr::~TraitExpr() {}

NmspExpr::NmspExpr(const feder::lexer::Position &pos, const std::string &name,
                   std::unique_ptr<Program> &&program) noexcept
    : IdExpr(expr_nmsp, pos, name), program(std::move(program)) {}

NmspExpr::~NmspExpr() {}

BiOpExpr::BiOpExpr(const feder::lexer::Position &pos,
                   feder::lexer::OperatorType opType,
                   std::unique_ptr<Expr> &&lhs,
                   std::unique_ptr<Expr> &&rhs) noexcept
    : Expr(expr_biop, pos), opType{opType}, lhs(std::move(lhs)),
      rhs(std::move(rhs)) {
#ifdef SANITY
  FEDER_SANITY_CHECK((bool) this->lhs && (bool) this->rhs,
      "lhs and rhs not optional.");
#endif
}

BiOpExpr::~BiOpExpr() {}

UnOpExpr::UnOpExpr(const feder::lexer::Position &pos,
                   feder::lexer::OperatorPosition opPos,
                   feder::lexer::OperatorType opType,
                   std::unique_ptr<Expr> &&expr) noexcept
    : Expr(expr_unop, pos), opPos{opPos}, opType{opType},
      expr(std::move(expr)) {
#ifdef SANITY
  FEDER_SANITY_CHECK((bool) this->expr, "expr not optional.");
#endif
}

UnOpExpr::~UnOpExpr() {}

BraceExpr::BraceExpr(const feder::lexer::Position &pos,
                     std::unique_ptr<Expr> &&expr) noexcept
    : Expr(expr_brace, pos), expr(std::move(expr)) {}

BraceExpr::~BraceExpr() {}

TemplateExpr::TemplateExpr(
    const lexer::Position &pos,
    std::vector<std::unique_ptr<Expr>> &&templates) noexcept
    : Expr(expr_template, pos), templates(std::move(templates)) {
#ifdef SANITY
  FEDER_SANITY_CHECK(this->templates.size() > 0, "templates.size() == 0");
#endif
}

TemplateExpr::~TemplateExpr() {}

ArrayConExpr::ArrayConExpr(const feder::lexer::Position &pos,
                           std::unique_ptr<Expr> &&obj,
                           std::unique_ptr<Expr> &&size) noexcept
    : Expr(expr_array_con, pos), obj(std::move(obj)), size(std::move(size)) {
#ifdef SANITY
  FEDER_SANITY_CHECK((bool) this->obj && (bool) this->size,
      "obj and size not optional.");
#endif
}

ArrayConExpr::~ArrayConExpr() {}

ArrayListExpr::ArrayListExpr(const feder::lexer::Position &pos,
                             std::vector<std::unique_ptr<Expr>> &&objs) noexcept
    : Expr(expr_array_list, pos), objs(std::move(objs)) {
#ifdef SANITY
  FEDER_SANITY_CHECK(this->objs.size() > 1, "objs.size() <= 1");
#endif
}

ArrayListExpr::~ArrayListExpr() {}

ArrayIndexExpr::ArrayIndexExpr(const feder::lexer::Position &pos,
                               std::unique_ptr<Expr> &&indexExpr) noexcept
    : Expr(expr_array_index, pos), indexExpr(std::move(indexExpr)) {
#ifdef SANITY
  FEDER_SANITY_CHECK((bool) this->indexExpr, "indexExpr not optional.");
#endif
}

ArrayIndexExpr::~ArrayIndexExpr() {}

IfExpr::IfExpr(const lexer::Position &pos, std::vector<IfCaseExpr> &&ifs,
               std::unique_ptr<Program> &&elseExpr) noexcept
    : Expr(expr_if, pos), ifs(std::move(ifs)), elseExpr(std::move(elseExpr)) {
#ifdef SANITY
  FEDER_SANITY_CHECK(this->ifs.size() > 0, "ifs.size() == 0");
  for (auto &ifcase : this->ifs) {
    FEDER_SANITY_CHECK((bool) ifcase.first, "!ifcase.first");
    FEDER_SANITY_CHECK((bool) ifcase.second, "!ifcase.second");
  }
#endif
}

IfExpr::~IfExpr() {}

MatchExpr::MatchExpr(const lexer::Position &pos,
                     std::unique_ptr<Expr> &&enumVal,
                     std::vector<MatchCaseExpr> &&matchCases,
                     std::unique_ptr<Program>   &&defaultCase) noexcept
    : Expr(expr_match, pos), enumVal(std::move(enumVal)),
      matchCases(std::move(matchCases)),
      defaultCase(std::move(defaultCase)) {
#ifdef SANITY
  FEDER_SANITY_CHECK((bool) this->enumVal, "enumVal not optional.");
  FEDER_SANITY_CHECK((bool) this->defaultCase || this->matchCases.size() > 0,
      "At least one case expected.");
#endif
}

MatchExpr::~MatchExpr() {}

ForExpr::ForExpr(const lexer::Position &pos,
                 std::unique_ptr<syntax::Expr> &&initExpr,
                 std::unique_ptr<syntax::Expr> &&condExpr,
                 std::unique_ptr<syntax::Expr> &&stepExpr,
                 std::unique_ptr<syntax::Program> &&program,
                 bool postConditionCheck) noexcept
    : Expr(expr_for, pos),
      initExpr(std::move(initExpr)), condExpr(std::move(condExpr)),
      stepExpr(std::move(stepExpr)), program(std::move(program)),
      postConditionCheck{postConditionCheck} {
#ifdef SANITY
  FEDER_SANITY_CHECK((bool) this->condExpr, "condExpr not optional.");
  FEDER_SANITY_CHECK((bool) this->program, "program not optional.");
#endif
}

ForExpr::~ForExpr() {}

CapsExpr::CapsExpr(const lexer::Position &pos, std::uint32_t bitmap) noexcept
	: Expr(expr_caps, pos), bitmap{bitmap} {}

CapsExpr::~CapsExpr() {}

// reportSyntaxError

std::unique_ptr<Expr>
feder::syntax::reportSyntaxError(feder::lexer::Tokenizer &lex,
                                 const feder::lexer::Position &pos,
                                 const std::string &msg) noexcept {
  lex.reportSyntaxError(msg, pos);
  return nullptr;
}

// to_string

std::string NumExpr::to_string() const noexcept {
  switch (getNumberType()) {
  case lexer::num_i8:
    return std::to_string(getNumberValue().i8);
  case lexer::num_i16:
    return std::to_string(getNumberValue().i16);
  case lexer::num_i32:
    return std::to_string(getNumberValue().i32);
  case lexer::num_i64:
    return std::to_string(getNumberValue().i64);

  case lexer::num_u8:
    return std::to_string(getNumberValue().u8);
  case lexer::num_u16:
    return std::to_string(getNumberValue().u16);
  case lexer::num_u32:
    return std::to_string(getNumberValue().u32);
  case lexer::num_u64:
    return std::to_string(getNumberValue().u64);

  case lexer::num_f32:
    return std::to_string(getNumberValue().f32);
  case lexer::num_f64:
    return std::to_string(getNumberValue().f64);
  }

  feder::fatal("Impossible-to-reach code reached");
  return "";
}

std::string StrExpr::to_string() const noexcept { return "\"" + str + "\""; }

std::string CharExpr::to_string() const noexcept {
  return std::string("\'") + c + "\'";
}

std::string FuncParamExpr::to_string() const noexcept {
  std::string result = getIdentifier() + " : " + getSemanticType().to_string();

  if (hasGuard()) {
    result += " | " + getGuard().to_string();
    if (hasGuardResult())
      result += " => " + getGuardResult().to_string();
  }

  return result;
}

std::string FuncExpr::to_string() const noexcept {
  std::string result = "func";
  if (hasTemplate())
    result += getTemplate().to_string();

  if (getName().size() > 0) {
    bool isfirst = true;
    for (const std::string &name : getName()) {
      if (isfirst) {
        result += " " + name;
        isfirst = false;
      } else {
        result += "." + name;
      }
    }
  }

  if (getParameters().size() > 0) {
    result += "(";
    for (auto &param : getParameters()) {
      if (result[result.size() - 1] != '(')
        result += ", ";

      result += param->to_string();
    }
    result += ")";
  }

  if (hasReturnType()) {
    result += " : ";
    result += getReturnType().to_string();
  }

  return result;
}

std::string ClassExpr::to_string() const noexcept {
  std::string result = "class";
  if (hasTemplate())
    result += getTemplate().to_string();

  result += " " + getIdentifier();

  if (getTraits().size() > 0) {
    result += " : ";
    for (auto &t : getTraits()) {
      if (result[result.size() - 2] != ':')
        result += ", ";

      result += t->to_string();
    }
  }

  return result;
}

std::string EnumExpr::to_string() const noexcept {
  std::string result = "enum";
  if (hasTemplate())
    result += getTemplate().to_string();

  result += " " + getIdentifier();

  return result;
}

std::string TraitExpr::to_string() const noexcept {
  std::string result = "trait";
  if (hasTemplate())
    result += getTemplate().to_string();

  result += " " + getIdentifier();

  if (getTraits().size() > 0) {
    result += " : ";
    for (auto &t : getTraits()) {
      if (result[result.size() - 2] != ':')
        result += ", ";

      result += t->to_string();
    }
  }

  return result;
}

std::string NmspExpr::to_string() const noexcept {
  return "namespace " + getIdentifier();
}

std::string BiOpExpr::to_string() const noexcept {
  switch (getOperator()) {
  case lexer::op_fncall:
  case lexer::op_indexcall:
  case lexer::op_templatecall:
    return getLHS().to_string() + getRHS().to_string();
  default:
    return "(" + getLHS().to_string() + " " + std::to_string(getOperator()) +
           " " + getRHS().to_string() + ")";
  }
}

std::string UnOpExpr::to_string() const noexcept {
  switch (getOperatorPosition()) {
  case lexer::op_lunary:
    return "(" + std::to_string(getOperator()) + getExpression().to_string() +
           ")";
  case lexer::op_runary:
    return "(" + getExpression().to_string() + std::to_string(getOperator()) +
           ")";
  }

  feder::fatal("Impossible-to-reach code reached");
  return "";
}

std::string BraceExpr::to_string() const noexcept {
  if (!hasExpression())
    return std::string("()");
  return "(" + getExpression().to_string() + ")";
}

std::string TemplateExpr::to_string() const noexcept {
  std::string result = "{";

  for (auto &templ : getTemplates()) {
    if (result[result.size() - 1] != '{')
      result += ", ";

    result += templ->to_string();
  }

  result += "}";
  return result;
}

std::string ArrayConExpr::to_string() const noexcept {
  return "[" + getObject().to_string() + " ; " + getSize().to_string() + "]";
}

std::string ArrayListExpr::to_string() const noexcept {
  std::string result = "[";
  for (auto &obj : getObjects()) {
    if (result[result.size() - 1] != '[')
      result += ", ";

    result += obj->to_string();
  }

  result += "]";

  return result;
}

std::string ArrayIndexExpr::to_string() const noexcept {
  return "[" + getIndex().to_string() + "]";
}

std::string IfExpr::to_string() const noexcept {
  std::string result;
  for (auto it = getIfCases().begin(); it != getIfCases().end(); ++it) {
    if (it != getIfCases().begin())
      result += "else ";

    result += "if ";

    result += it->first->to_string();
    result += "\n";
  }

  if (hasElseCase()) {
    result += "else\n";
  }

  result += ";";

  return result;
}

std::string MatchExpr::to_string() const noexcept {
  std::string result;
  result += "match " + getEnumValue().to_string();

  return result;
}

std::string ForExpr::to_string() const noexcept {
  std::string result;
  result += "for ";

  if (hasInitialization()) {
    result += getInitialization().to_string();
    result += "; ";
  }

  result += getCondition().to_string();

  if (hasStep()) {
    result += "; ";
    result += getStep().to_string();
  }

  return result;
}

std::string CapsExpr::to_string() const noexcept {
  std::string result;

  if ((bitmap & CAPS_SAFE) != 0)
    result += "Safe";
  
  if ((bitmap & CAPS_CONST) != 0) {
    if (result.length() > 0)
      result += ", ";
    result += "Const";
  }
  
  if ((bitmap & CAPS_THIS) != 0) {
    if (result.length() > 0)
      result += ", ";
    result += "This";
  }

  return result;
}

// isStatement

bool FuncExpr::isStatement() const noexcept {
  if (isType())
    return false;
  if (getName().back() == "_")
    return false;

  return true;
}

bool ClassExpr::isStatement() const noexcept { return getIdentifier() != "_"; }

bool EnumExpr::isStatement() const noexcept { return getIdentifier() != "_"; }

bool TraitExpr::isStatement() const noexcept { return getIdentifier() != "_"; }

bool BiOpExpr::isStatement() const noexcept {
  switch (getOperator()) {
  case lexer::op_def:
  case lexer::op_asg:
  case lexer::op_asg_band:
  case lexer::op_asg_bxor:
  case lexer::op_asg_bor:
  case lexer::op_asg_add:
  case lexer::op_asg_sub:
  case lexer::op_asg_mul:
  case lexer::op_asg_div:
  case lexer::op_asg_mod:
  case lexer::op_asg_lsh:
  case lexer::op_asg_rsh:
  case lexer::op_decl:
  case lexer::op_fncall:
    return true;
  default:
    return false;
  }
}

bool NmspExpr::isStatement() const noexcept { return getIdentifier() != "_"; }

bool IfExpr::isStatement() const noexcept { return true; }

bool MatchExpr::isStatement() const noexcept { return true; }

bool ForExpr::isStatement() const noexcept { return true; }

// hasReturn

bool IdExpr::hasReturn() const noexcept {
  return getIdentifier() != "_";
}

bool NumExpr::hasReturn() const noexcept {
  return true;
}

bool StrExpr::hasReturn() const noexcept {
  return true;
}

bool CharExpr::hasReturn() const noexcept {
  return true;
}

bool FuncExpr::hasReturn() const noexcept {
  return getName().size() == 0
    || (getName().size() == 1 && getName()[0] == "_");
}

bool ClassExpr::hasReturn() const noexcept {
  return getIdentifier() == "_";
}

bool BiOpExpr::hasReturn() const noexcept {
  if (getOperator() == lexer::op_fncall)
    return getLHS().hasReturn();

  return getLHS().hasReturn() && getRHS().hasReturn();
}

bool UnOpExpr::hasReturn() const noexcept {
  return getExpression().hasReturn();
}

bool BraceExpr::hasReturn() const noexcept {
  return hasExpression() && getExpression().hasReturn();
}

bool ArrayConExpr::hasReturn() const noexcept {
  return getObject().hasReturn() && getSize().hasReturn();
}

bool ArrayListExpr::hasReturn() const noexcept {
  for (auto &obj : getObjects()) {
    if (!obj->hasReturn())
      return false;
  }

  return true;
}

bool ArrayIndexExpr::hasReturn() const noexcept {
  return getIndex().hasReturn();
}

bool IfExpr::hasReturn() const noexcept {
  if (!hasElseCase())
    return false;

  for (auto &ifcase : getIfCases()) {
    if (!ifcase.second->hasReturn())
      return false;
  }

  return getElseCase().hasReturn();
}

bool MatchExpr::hasReturn() const noexcept {
  if (!hasDefaultCase())
    return false;

  for (auto &matchcase : getMatchCases()) {
    if (!matchcase.second->hasReturn())
      return false;
  }

  return getDefaultCase().hasReturn();
}
