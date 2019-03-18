#include "feder/syntax.hpp"
using namespace feder::syntax;

Program::Program(std::vector<std::unique_ptr<Expr>> lines) noexcept
    : lines(std::move(lines)) {
}

Program::~Program() {
}

Expr::Expr(ExprType type, const feder::lexer::Position &pos) noexcept
    : type{type}, pos(pos) {
}

Expr::~Expr() {
}

IdExpr::IdExpr(ExprType type, const feder::lexer::Position &pos,
    const std::string &id) noexcept
    : Expr(type, pos), id(id) {
}

IdExpr::IdExpr(const feder::lexer::Position &pos,
    const std::string &id) noexcept
    : Expr(expr_id, pos), id(id) {
}

IdExpr::~IdExpr() {
}

NumExpr::NumExpr(const feder::lexer::Position &pos,
    feder::lexer::NumberType numType,
    feder::lexer::NumberValue numVal) noexcept
    : Expr(expr_num, pos), numType{numType}, numVal{numVal} {
}

NumExpr::~NumExpr() {
}

StrExpr::StrExpr(const feder::lexer::Position &pos,
    const std::string &str) noexcept
    : Expr(expr_str, pos), str(str) {
}

StrExpr::~StrExpr() {
}

CharExpr::CharExpr(const feder::lexer::Position &pos, char c) noexcept
    : Expr(expr_char, pos), c{c} {
}

CharExpr::~CharExpr() {
}

FuncParamExpr::FuncParamExpr(const feder::lexer::Position &pos,
    const std::string &name,
    std::unique_ptr<Expr> semanticType,
    std::unique_ptr<Expr> guard,
    std::unique_ptr<Expr> guardResult) noexcept
    : IdExpr(expr_func_param, pos, name),
      semanticType(std::move(semanticType)),
      guard(std::move(guard)),
      guardResult(std::move(guardResult)) {
}

FuncParamExpr::~FuncParamExpr() {
}

FuncExpr::FuncExpr(const feder::lexer::Position &pos,
    const std::string &name,
    std::unique_ptr<Expr> returnType,
    std::vector<std::unique_ptr<Expr>> params,
    std::unique_ptr<Program> program) noexcept
    : IdExpr(expr_func, pos, name),
      returnType(std::move(returnType)),
      params(std::move(params)),
      program(std::move(program)) {
}

FuncExpr::~FuncExpr() {
}

ClassExpr::ClassExpr(const feder::lexer::Position &pos,
    const std::string &name,
    std::vector<std::unique_ptr<Expr>> attributes,
    std::vector<std::unique_ptr<FuncExpr>> functions) noexcept
    : IdExpr(expr_class, pos, name),
      attributes(std::move(attributes)),
      functions(std::move(functions)) {
}

ClassExpr::~ClassExpr() {
}

EnumExpr::EnumExpr(const feder::lexer::Position &pos,
    const std::string &name,
    std::vector<std::unique_ptr<BiOpExpr>> constructors) noexcept
    : IdExpr(expr_enum, pos, name),
      constructors(std::move(constructors)) {
}

EnumExpr::~EnumExpr() {
}

TraitExpr::TraitExpr(const feder::lexer::Position &pos,
    const std::string &name,
    std::vector<std::unique_ptr<FuncExpr>> functions) noexcept
    : IdExpr(expr_trait, pos, name),
      functions(std::move(functions)) {
}

TraitExpr::~TraitExpr() {
}

NmspExpr::NmspExpr(const feder::lexer::Position &pos,
    const std::string &name, std::unique_ptr<Program> program) noexcept
    : IdExpr(expr_nmsp, pos, name), program(std::move(program)) {
}

NmspExpr::~NmspExpr() {
}

BiOpExpr::BiOpExpr(const feder::lexer::Position &pos,
    feder::lexer::OperatorType opType,
    std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs) noexcept
    : Expr(expr_biop, pos), opType{opType},
      lhs(std::move(lhs)), rhs(std::move(rhs)) {
}

BiOpExpr::~BiOpExpr() {
}

UnOpExpr::UnOpExpr(const feder::lexer::Position &pos,
    feder::lexer::OperatorPosition opPos,
    feder::lexer::OperatorType opType,
    std::unique_ptr<Expr> expr) noexcept
    : Expr(expr_unop, pos), opPos{opPos}, opType{opType},
      expr(std::move(expr)) {
}

UnOpExpr::~UnOpExpr() {
}
