#ifndef FEDER_SYNTAX_HPP
#define FEDER_SYNTAX_HPP

/*!\file func/syntax.hpp
 * \brief Syntax Elements
 */

#include "feder/global.hpp"
#include "feder/lexer.hpp"

/*!\file feder/syntax.hpp
 * \brief Syntax expressions.
 */

/*!\defgroup syntax Syntactic analysis results
 */

namespace feder {
namespace syntax {
/*!\addtogroup syntax
 * \brief Contains syntactic elements required by the parser.
 *
 * \{
 */

class Program;
class Expr;
class IdExpr;
class NumExpr;
class StrExpr;
class CharExpr;
class FuncParamExpr;
class FuncExpr;
class ClassExpr;
class EnumExpr;
class TraitExpr;
class NmspExpr;
class BiOpExpr;
class UnOpExpr;
class BraceExpr;
class TemplateExpr;
class IfExpr;
class MatchExpr;
class ForExpr;

class Program {
  std::vector<std::unique_ptr<Expr>> lines;
  std::unique_ptr<Expr> returnExpr; //!< Optional
  bool error;

public:
  Program(std::vector<std::unique_ptr<Expr>> &&lines,
          std::unique_ptr<Expr> &&returnExpr,
          bool error = false) noexcept;
  virtual ~Program();

  /*!\return Returns lines of program.
   */
  auto &getLines() noexcept { return lines; }

  /*!\return Returns lines of program (const).
   */
  const auto &getLines() const noexcept { return lines; }

  /*!\return Returns true, if program has error, otherwise false.
   */
  bool hasError() const noexcept { return error; }

  /*!\return Returns true, if program has return. Otherwise false is returned.
   */
  bool hasReturn() const noexcept { return (bool) returnExpr; }

  /*!\return Returns optional return expression.
   */
  auto &getReturnExpr() noexcept
  { return *returnExpr; }

  /*!\return Returns optional return expression (const).
   */
  const auto &getReturnExpr() const noexcept
  { return *returnExpr; }
};

/*!\brief Expression types.
 * \see Expr
 */
enum ExprType {
  expr_id,         //!< \see IdExpr
  expr_num,        //!< \see NumExpr
  expr_func,       //!< \see FuncExpr
  expr_func_param, //!< \see FuncParamExpr
  expr_str,        //!< \see StrExpr
  expr_char,       //!< \see CharExpr
  expr_class,      //!< \see ClassExpr
  expr_enum,       //!< \see EnumExpr
  expr_trait,      //!< \see TraitExpr
  expr_nmsp,       //!< \see NmspExpr

  expr_if,    //!< \see IfExpr
  expr_match, //!< \see MatchExpr
  expr_for,   //!< \see ForExpr

  expr_array_con,   //!< \see ArrayConExpr
  expr_array_list,  //!< \see ArrayListExpr
  expr_array_index, //!< \see ArrayIndexExpr

  expr_unop, //!< \see UnOpExpr
  expr_biop, //!< \see BiOpExpr

  expr_brace,    //!< \see BraceExpr
  expr_template, //!< \see TemplateExpr
};

/*!\brief Parent of expressions.
 */
class Expr {
  ExprType type;
  lexer::Position pos;

public:
  /*!\brief Initializes expression with type and in-code position.
   * \param type
   * \param pos
   */
  Expr(ExprType type, const lexer::Position &pos) noexcept;
  virtual ~Expr();

  /*!\return Returns expression type.
   */
  ExprType getType() const noexcept { return type; }

  /*!\return Returns in-code position.
   */
  const lexer::Position &getPosition() const noexcept { return pos; }

  /*!\return Returns expression converted to Feder code.
   */
  virtual std::string to_string() const noexcept {
    feder::fatal("Compiler error");
    return std::string("");
  }

  /*!\return Returns true, if this expression is a statements, otherwise
   * this expression is a secondary statement and false is returned.
   */
  virtual bool isStatement() const noexcept { return false; }

  /*!\return Returns getType() == type.
   * \param type
   */
  bool operator ==(ExprType type) const noexcept { return getType() == type; }

  /*!\return Returns getType() != type.
   * \param type
   */
  bool operator !=(ExprType type) const noexcept { return getType() != type; }
};

/*!\brief Identifier expression.
 */
class IdExpr : public Expr {
  std::string id;

protected:
  IdExpr(ExprType type, const lexer::Position &pos,
         const std::string &id) noexcept;

public:
  /*!\brief Initializes identifier expression with id (identifier).
   */
  IdExpr(const lexer::Position &pos, const std::string &id) noexcept;
  virtual ~IdExpr();

  /*!\return Returns associated identifier.
   */
  const std::string &getIdentifier() const noexcept { return id; }

  virtual std::string to_string() const noexcept override { return id; }
};

/*!\brief Expression for floating-point and integer numberse.
 */
class NumExpr : public Expr {
  lexer::NumberValue numVal;
  lexer::NumberType numType;

public:
  /*!\brief Initializes number expression.
   */
  NumExpr(const lexer::Position &pos, lexer::NumberType numType,
          lexer::NumberValue numVal) noexcept;
  virtual ~NumExpr();

  /*!\return Returns value of the number expression.
   * \see getNumberType
   */
  auto getNumberValue() const noexcept { return numVal; }

  /*!\return Returns type of the number.
   * \see getNumber
   */
  auto getNumberType() const noexcept { return numType; }

  virtual std::string to_string() const noexcept override;
};

/*!\brief String expression
 */
class StrExpr : public Expr {
  std::string str;

public:
  StrExpr(const lexer::Position &pos, const std::string &str) noexcept;
  virtual ~StrExpr();

  /*!\return Returns associated string.
   */
  const std::string &getString() const noexcept { return str; }

  virtual std::string to_string() const noexcept override;
};

/*!\brief Character expression
 */
class CharExpr : public Expr {
  char c;

public:
  CharExpr(const lexer::Position &pos, char c) noexcept;
  virtual ~CharExpr();

  /*!\return Returns associated character.
   */
  char getCharacter() const noexcept { return c; }

  virtual std::string to_string() const noexcept override;
};

/*!\brief Function parameter expression.
 *
 *     <func-param> := <id> ';' <type>
 *                   | <id> ':' <type> <guard>
 *     <guard>      := '|' <expr>
 *                   | '|' <expr> '=>' <expr>
 */
class FuncParamExpr : public IdExpr {
  std::unique_ptr<Expr> semanticType;
  std::unique_ptr<Expr> guard;       //!< Can be null (optional).
  std::unique_ptr<Expr> guardResult; //!< Can be null (optional).
public:
  /*!\brief Initializes function parameter expression.
   * \param name Name of the parameter
   * \param semanticType
   * \param guard Can be null (optional).
   * \param guardResult Can be null (optional).
   */
  FuncParamExpr(const lexer::Position &pos, const std::string &name,
                std::unique_ptr<Expr> &&semanticType,
                std::unique_ptr<Expr> &&guard,
                std::unique_ptr<Expr> &&guardResult) noexcept;
  virtual ~FuncParamExpr();

  /*!\return Returns the semantic type of the parameter.
   */
  Expr &getSemanticType() noexcept { return *semanticType; }

  /*!\return Returns the semantic type of the parameter (const).
   */
  const Expr &getSemanticType() const noexcept { return *semanticType; }

  /*!\return Returns optional guard.
   * \see hasGuard
   */
  Expr &getGuard() noexcept { return *guard; }

  /*!\return Returns optional guard (const).
   * \see hasGuardResult
   */
  const Expr &getGuard() const noexcept { return *guard; }

  /*!\return Returns optional guard result. Can be null (optional).
   */
  Expr &getGuardResult() noexcept { return *guardResult; }

  /*!\return Returns optional guard result (const). Can be null (optional).
   */
  const Expr &getGuardResult() const noexcept { return *guardResult; }

  bool hasGuard() const noexcept { return (bool)guard; }

  bool hasGuardResult() const noexcept { return (bool)guardResult; }

  virtual std::string to_string() const noexcept override;
};

/*!\brief Function expression.
 */
class FuncExpr : public Expr {
  std::unique_ptr<Expr> returnType; //!< Can be null (optional).
  std::vector<std::unique_ptr<FuncParamExpr>> params;
  std::unique_ptr<TemplateExpr> templ;

  std::unique_ptr<Program> program;

  std::vector<std::string> name;

  bool virtualFunc;

public:
  FuncExpr(const lexer::Position &pos, const std::vector<std::string> &name,
           std::unique_ptr<TemplateExpr> &&templ,
           std::unique_ptr<Expr> &&returnType,
           std::vector<std::unique_ptr<FuncParamExpr>> &&params,
           std::unique_ptr<Program> &&program, bool virtualFunc) noexcept;
  virtual ~FuncExpr();

  /*!\return Returns name of function. If empty, not a func declaration,
   * but function type.
   */
  const auto &getName() const noexcept { return name; }

  bool isType() const noexcept { return name.size() == 0; }

  /*!\return Returns true, if function is a virtual function
   * (starting with Func). Otherwise false is returned.
   */
  bool isVirtual() const noexcept { return virtualFunc; }

  bool hasTemplate() const noexcept { return (bool)templ; }

  auto &getTemplate() noexcept { return *templ; }

  const auto &getTemplate() const noexcept { return *templ; }

  /*!\return Returns return type of the function. Can be null (optional).
   * \see hasReturnType
   */
  Expr &getReturnType() noexcept { return *returnType; }

  /*!\return Returns return type of the function (const).
   * \see hasReturnType
   */
  const Expr &getReturnType() const noexcept { return *returnType; }

  bool hasReturnType() const noexcept { return (bool)returnType; }

  /*!\return Returns parameters of function.
   */
  auto &getParameters() noexcept { return params; }

  /*!\return Returns parameters of function (const).
   */
  const auto &getParameters() const noexcept { return params; }

  /*!\return Returns function body. Can be null (optional).
   * \see isDeclared isDefined
   */
  Program &getProgram() noexcept { return *program; }

  /*!\return Returns function body (const). Can be null (optional).
   * \see isDeclared isDefined
   */
  const Program &getProgram() const noexcept { return *program; }

  /*!\return Returns true, if only declared, otherwise false.
   */
  bool isDeclared() const noexcept {
    return !program && !isType();
  } // If no program => Declared

  /*!\return Returns true, if defined function, otherwise false.
   */
  bool isDefined() const noexcept { return (bool)program && !isType(); }

  virtual std::string to_string() const noexcept override;

  virtual bool isStatement() const noexcept override;
};

/*!\brief Class expression.
 */
class ClassExpr : public IdExpr {
  std::unique_ptr<TemplateExpr> templ;
  std::vector<std::unique_ptr<Expr>> traits;
  std::vector<std::unique_ptr<Expr>> attributes;
  std::vector<std::unique_ptr<FuncExpr>> constructors;
  std::vector<std::unique_ptr<FuncExpr>> functions;

public:
  ClassExpr(const lexer::Position &pos, const std::string &name,
            std::unique_ptr<TemplateExpr> &&templ,
            std::vector<std::unique_ptr<Expr>> &&traits,
            std::vector<std::unique_ptr<Expr>> &&attributes,
            std::vector<std::unique_ptr<FuncExpr>> &&constructors,
            std::vector<std::unique_ptr<FuncExpr>> &&functions) noexcept;
  virtual ~ClassExpr();

  bool hasTemplate() const noexcept { return (bool)templ; }

  auto &getTemplate() noexcept { return *templ; }

  const auto &getTemplate() const noexcept { return *templ; }

  /*!\return Returns inherited traits.
   */
  auto &getTraits() noexcept { return traits; }

  /*!\return Returns inherited traits (const).
   */
  const auto &getTraits() const noexcept { return traits; }

  /*!\return Returns (potential) attributes of the class.
   */
  auto &getAttributes() noexcept { return attributes; }

  /*!\return Returns (potential) attributes of the class (const).
   */
  const auto &getAttributes() const noexcept { return attributes; }

  /*!\return Returns constructors.
   */
  auto &getConstructors() noexcept { return constructors; }

  /*!\return Returns constructors (const).
   */
  const auto &getConstructors() const noexcept { return constructors; }

  /*!\return Returns functions of this class (not constructors).
   */
  auto &getFunctions() noexcept { return functions; }

  /*!\return Returns functions of this class (not constructors).
   */
  const auto &getFunctions() const noexcept { return functions; }

  virtual std::string to_string() const noexcept override;

  virtual bool isStatement() const noexcept override;
};

/*!\brief Enum expression.
 */
class EnumExpr : public IdExpr {
  std::vector<std::unique_ptr<Expr>> constructors;
  std::unique_ptr<TemplateExpr> templ;

public:
  EnumExpr(const lexer::Position &pos, const std::string &name,
           std::unique_ptr<TemplateExpr> &&templ,
           std::vector<std::unique_ptr<Expr>> &&constructors) noexcept;
  virtual ~EnumExpr();

  bool hasTemplate() const noexcept { return (bool)templ; }

  auto &getTemplate() noexcept { return *templ; }

  const auto &getTemplate() const noexcept { return *templ; }

  /*!\return Returns constructors of the enum.
   */
  auto &getConstructors() noexcept { return constructors; }

  /*!\return Returns constructors of the enum (const).
   */
  const auto &getConstructors() const noexcept { return constructors; }

  virtual std::string to_string() const noexcept override;

  virtual bool isStatement() const noexcept override;
};

/*!\brief Trait expression
 */
class TraitExpr : public IdExpr {
  std::unique_ptr<TemplateExpr> templ;
  std::vector<std::unique_ptr<Expr>> traits;
  std::vector<std::unique_ptr<FuncExpr>> functions;

public:
  /*!\brief Initialize instance of TraitExpr.
   * \param name Name of the trait.
   * \param functions Declared (not defined) functions.
   */
  TraitExpr(const lexer::Position &pos, const std::string &name,
            std::unique_ptr<TemplateExpr> &&templ,
            std::vector<std::unique_ptr<Expr>> &&traits,
            std::vector<std::unique_ptr<FuncExpr>> &&functions) noexcept;
  virtual ~TraitExpr();

  bool hasTemplate() const noexcept { return (bool)templ; }

  auto &getTemplate() noexcept { return *templ; }

  /*!\return Returns inherited traits.
   */
  const auto &getTemplate() const noexcept { return *templ; }

  /*!\return Returns inherited traits.
   */
  auto &getTraits() noexcept { return traits; }

  /*!\return Returns inherited traits (const).
   */
  const auto &getTraits() const noexcept { return traits; }

  /*!\return Returns declared functions.
   */
  auto &getFunctions() noexcept { return functions; }

  /*!\return Returns declared functions (const).
   */
  const auto &getFunctions() const noexcept { return functions; }

  virtual std::string to_string() const noexcept override;

  virtual bool isStatement() const noexcept override;
};

/*!\brief Namespace expression.
 */
class NmspExpr : public IdExpr {
  std::unique_ptr<Program> program;

public:
  NmspExpr(const lexer::Position &pos, const std::string &name,
           std::unique_ptr<Program> &&program) noexcept;
  virtual ~NmspExpr();

  /*!\return Returns program of the namespace.
   */
  auto &getProgram() noexcept { return program; }

  /*!\return Returns program of the namespace.
   */
  const auto &getProgram() const noexcept { return program; }

  virtual std::string to_string() const noexcept override;

  virtual bool isStatement() const noexcept override;
};

/*!\brief Binary operator expression.
 */
class BiOpExpr : public Expr {
  lexer::OperatorType opType;
  std::unique_ptr<Expr> lhs, rhs;

public:
  BiOpExpr(const lexer::Position &pos, lexer::OperatorType opType,
           std::unique_ptr<Expr> &&lhs, std::unique_ptr<Expr> &&rhs) noexcept;
  virtual ~BiOpExpr();

  /*!\return Returns binary operator type.
   */
  lexer::OperatorType getOperator() const noexcept { return opType; }

  /*!\return Returns left-hand-side.
   */
  Expr &getLHS() noexcept { return *lhs; }

  /*!\return Returns rhs (as unique pointer). This makes RHS of this binary
   * operator invalid.
   */
  std::unique_ptr<Expr> moveRHS() noexcept { return std::move(rhs); }

  /*!\return Returns lhs (as unique pointer). This makes LHS of this binary
   * operator invalid.
   */
  std::unique_ptr<Expr> moveLHS() noexcept { return std::move(lhs); }

  /*!\return Returns left-hand-side (const).
   */
  const Expr &getLHS() const noexcept { return *lhs; }

  /*!\return Returns right-hand-side.
   */
  Expr &getRHS() noexcept { return *rhs; }

  /*!\return Returns right-hand-side (const).
   */
  const Expr &getRHS() const noexcept { return *rhs; }

  virtual std::string to_string() const noexcept override;

  virtual bool isStatement() const noexcept override;

  /*!\return Returns getOperator() == type.
   * \param type
   */
  bool operator ==(lexer::OperatorType type) const noexcept {
    return getOperator() == type;
  }

  /*!\return Returns getOperator() != type.
   * \param type
   */
  bool operator !=(lexer::OperatorType type) const noexcept {
    return getOperator() != type;
  }
};

/*!\brief Unary operator expression
 */
class UnOpExpr : public Expr {
  lexer::OperatorType opType;
  lexer::OperatorPosition opPos;
  std::unique_ptr<Expr> expr;

public:
  UnOpExpr(const lexer::Position &pos, lexer::OperatorPosition opPos,
           lexer::OperatorType opType, std::unique_ptr<Expr> &&expr) noexcept;
  virtual ~UnOpExpr();

  /*!\return Returns unary operator type.
   */
  lexer::OperatorType getOperator() const noexcept { return opType; }

  /*!\return Returns either op_lunary or op_runary.
   */
  lexer::OperatorPosition getOperatorPosition() const noexcept { return opPos; }

  /*!\return Returns expression associated with unary operator.
   */
  Expr &getExpression() noexcept { return *expr; }

  /*!\return Returns expression associated with unary operator (const).
   */
  const Expr &getExpression() const noexcept { return *expr; }

  virtual std::string to_string() const noexcept override;
};

/*!\brief Brace expression
 */
class BraceExpr : public Expr {
  std::unique_ptr<Expr> expr;

public:
  BraceExpr(const lexer::Position &pos, std::unique_ptr<Expr> &&expr) noexcept;
  virtual ~BraceExpr();

  bool hasExpression() const noexcept { return (bool)expr; }

  Expr &getExpression() noexcept { return *expr; }

  const Expr &getExpression() const noexcept { return *expr; }

  virtual std::string to_string() const noexcept override;
};

/*!\brief Template expression
 */
class TemplateExpr : public Expr {
  std::vector<std::unique_ptr<Expr>> templates;

public:
  TemplateExpr(const lexer::Position &pos,
               std::vector<std::unique_ptr<Expr>> &&templates) noexcept;
  virtual ~TemplateExpr();

  auto &getTemplates() noexcept { return templates; }

  const auto &getTemplates() const noexcept { return templates; }

  virtual std::string to_string() const noexcept override;
};

/*!\brief Array constructor expression
 */
class ArrayConExpr : public Expr {
  std::unique_ptr<Expr> obj;
  std::unique_ptr<Expr> size;

public:
  /*!\brief Initialize array constructor expression.
   * \param pos Position
   * \param obj The base (null) object.
   * \param size Size of the array.
   */
  ArrayConExpr(const lexer::Position &pos, std::unique_ptr<Expr> &&obj,
               std::unique_ptr<Expr> &&size) noexcept;
  virtual ~ArrayConExpr();

  /*!\return Returns the object to copy n times.
   */
  Expr &getObject() noexcept { return *obj; }

  /*!\return Returns the object to copy n times (const).
   */
  const Expr &getObject() const noexcept { return *obj; }

  /*!\return Returns the size expression.
   */
  Expr &getSize() noexcept { return *size; }

  /*!\return Returns the size expression (const).
   */
  const Expr &getSize() const noexcept { return *size; }

  virtual std::string to_string() const noexcept override;
};

/*!\brief Array list (constructor) expression.
 */
class ArrayListExpr : public Expr {
  std::vector<std::unique_ptr<Expr>> objs;

public:
  /*!\brief Initialize Array list expression.
   * \param pos Position
   * \param objs
   */
  ArrayListExpr(const lexer::Position &pos,
                std::vector<std::unique_ptr<Expr>> &&objs) noexcept;
  virtual ~ArrayListExpr();

  /*!\return Returns ordered list of objects the array should have.
   */
  std::vector<std::unique_ptr<Expr>> &getObjects() noexcept { return objs; }

  /*!\return Returns ordered list of objects the array should have (const).
   */
  const std::vector<std::unique_ptr<Expr>> &getObjects() const noexcept {
    return objs;
  }

  virtual std::string to_string() const noexcept override;
};

/*!\brief Array index expression.
 */
class ArrayIndexExpr : public Expr {
  std::unique_ptr<Expr> indexExpr;

public:
  ArrayIndexExpr(const lexer::Position &pos,
                 std::unique_ptr<Expr> &&indexExpr) noexcept;
  virtual ~ArrayIndexExpr();

  /*!\return Returns index expression.
   */
  Expr &getIndex() noexcept { return *indexExpr; }

  /*!\return Returns index expression (const).
   */
  const Expr &getIndex() const noexcept { return *indexExpr; }

  virtual std::string to_string() const noexcept override;
};

/*!\brief If case.
 */
typedef std::pair<std::unique_ptr<Expr>, std::unique_ptr<Program>> IfCaseExpr;

/*!\brief If expression.
 */
class IfExpr : public Expr {
  std::vector<IfCaseExpr> ifs;    //!< If cases. size >= 1.
  std::unique_ptr<Program> elseExpr; //!< Else case, optional.

public:
  IfExpr(const lexer::Position &pos, std::vector<IfCaseExpr> &&ifs,
         std::unique_ptr<Program> &&elseExpr) noexcept;

  virtual ~IfExpr();

  /*!\returns If cases. size >= 1.
   */
  auto &getIfCases() noexcept { return ifs; }

  /*!\returns If cases. size >= 1 (const).
   */
  const auto &getIfCases() const noexcept { return ifs; }

  /*!\return Returns true if has else case, otherwise false. If true is
   * returned [\ref getElseCase] is valid, otherwise invalid.
   */
  bool hasElseCase() const noexcept { return (bool)elseExpr; }

  /*!\return Returns else case. Optional, might be invalid.
   */
  auto &getElseCase() noexcept { return *elseExpr; }

  /*!\return Returns else case. Optional, might be invalid.
   */
  const auto &getElseCase() const noexcept { return *elseExpr; }

  virtual std::string to_string() const noexcept override;

  virtual bool isStatement() const noexcept override;
};

typedef std::pair<std::unique_ptr<Expr>, std::unique_ptr<Program>> MatchCaseExpr;

/*!\brief Match expression.
 */
class MatchExpr : public Expr {
  std::unique_ptr<Expr> enumVal;
  std::vector<MatchCaseExpr> matchCases;
public:
  MatchExpr(const lexer::Position &pos,
            std::unique_ptr<Expr> &&enumVal,
            std::vector<MatchCaseExpr> &&matchCases) noexcept;
  virtual ~MatchExpr();

  /*!\return Returns enum value.
   */
  auto &getEnumValue() noexcept { return *enumVal; }

  /*!\return Returns enum value (const).
   */
  const auto &getEnumValue() const noexcept { return *enumVal; }

  /*!\return Returns match cases.
   */
  auto &getMatchCases() noexcept { return matchCases; }

  /*!\return Returns match cases.
   */
  const auto &getMatchCases() const noexcept { return matchCases; }

  virtual std::string to_string() const noexcept override;

  virtual bool isStatement() const noexcept override;
};

/*!\brief For/Do expression
 */
class ForExpr : public Expr {
  bool postConditionCheck;
  std::unique_ptr<Expr> initExpr;
  std::unique_ptr<Expr> condExpr;
  std::unique_ptr<Expr> stepExpr;

  std::unique_ptr<Program> program;
public:
  /*!\brief Initializes for/do expression.
   * \param pos
   * \param initExpr Optional initialization expression.
   * \param condExpr Required loop-condition expression.
   * \param stepExpr Optional iterator step expression.
   * \param program
   * \param postConditionCheck True if 'do'-loop, false if 'for'-loop.
   */
  ForExpr(const lexer::Position &pos,
          std::unique_ptr<Expr> &&initExpr,
          std::unique_ptr<Expr> &&condExpr,
          std::unique_ptr<Expr> &&stepExpr,
          std::unique_ptr<Program> &&program,
          bool postConditionCheck = false) noexcept;

  virtual ~ForExpr();

  /*!\return Returns true, if condition is check post-body (so there's at
   * least one guaranteed executio of the body). This behaviour matches the
   * 'do'-loop. Otherwise false is returned, which correspondes to the
   * 'for'-loop.
   */
  bool isPostConditionCheck() const noexcept { return postConditionCheck; }

  /*!\return Returns if has initialization.
   */
  bool hasInitialization() const noexcept { return (bool) initExpr; }

  /*!\return Returns optional initialization expression.
   */
  auto &getInitialization() noexcept { return *initExpr; }

  /*!\return Returns optional initialization expression (const).
   */
  const auto &getInitialization() const noexcept { return *initExpr; }

  /*!\return Return loop condition.
   * \see isPostConditionCheck
   */
  auto &getCondition() noexcept { return *condExpr; }

  /*!\return Return loop condition (const).
   * \see isPostConditionCheck
   */
  const auto &getCondition() const noexcept { return *condExpr; }

  /*!\return Returns if has iterator step.
   */
  bool hasStep() const noexcept { return (bool) stepExpr; }

  /*!\return Returns optional iterator step.
   */
  auto &getStep() noexcept { return *stepExpr; }

  /*!\return Returns optional iterator step (const).
   */
  const auto &getStep() const noexcept { return *stepExpr; }

  /*!\return Returns program/body of the do/for loop.
   */
  auto &getProgram() noexcept { return *program; }
  
  /*!\return Returns program/body of the do/for loop.
   */
  const auto &getProgram() const noexcept { return *program; }

  virtual std::string to_string() const noexcept override;

  virtual bool isStatement() const noexcept override;
};

/*!\brief Prints error.
 * \return Returns nullptr;
 */
std::unique_ptr<Expr> reportSyntaxError(lexer::Lexer &lexer,
                                        const lexer::Position &pos,
                                        const std::string &msg) noexcept;

/*!\} */
} // end namespace syntax
} // end namespace feder

#endif /* FEDER_SYNTAX_HPP */
