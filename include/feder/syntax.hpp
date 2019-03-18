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

namespace feder {
  namespace syntax {
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

    class Program {
      std::vector<std::unique_ptr<Expr>> lines;
    public:
      Program(std::vector<std::unique_ptr<Expr>> lines) noexcept;
      virtual ~Program();

      /*!\return Returns lines of program.
       */
      auto &getLines() noexcept
      { return lines; }

      /*!\return Returns lines of program (const).
       */
      const auto &getLines() const noexcept
      { return lines; }
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
      ExprType getType() const noexcept
      { return type; }

      /*!\return Returns in-code position.
       */
      const lexer::Position &getPosition() const noexcept
      { return pos; }
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
      const std::string &getIdentifier() const noexcept
      { return id; }
    };

    /*!\brief Expression for floating-point and integer numberse.
     */
    class NumExpr : public Expr {
      lexer::NumberValue numVal;
      lexer::NumberType numType;
    public:
      /*!\brief Initializes number expression.
       */
      NumExpr(const lexer::Position &pos,
          lexer::NumberType numType,
          lexer::NumberValue numVal) noexcept;
      virtual ~NumExpr();

      /*!\return Returns value of the number expression.
       * \see getNumberType
       */
      auto getNumberValue() const noexcept
      { return numVal; }

      /*!\return Returns type of the number.
       * \see getNumber
       */
      auto getNumberType() const noexcept
      { return numType; }
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
      const std::string &getString() const noexcept
      { return str; }
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
      char getCharacter() const noexcept
      { return c; }
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
      std::unique_ptr<Expr> guard; //!< Can be null (optional).
      std::unique_ptr<Expr> guardResult; //!< Can be null (optional).
    public:
      /*!\brief Initializes function parameter expression.
       * \param name Name of the parameter
       * \param semanticType
       * \param guard Can be null (optional).
       * \param guardResult Can be null (optional).
       */
      FuncParamExpr(const lexer::Position &pos, const std::string &name,
          std::unique_ptr<Expr> semanticType,
          std::unique_ptr<Expr> guard,
          std::unique_ptr<Expr> guardResult) noexcept;
      virtual ~FuncParamExpr();

      /*!\return Returns the semantic type of the parameter.
       */
      Expr &getSemanticType() noexcept
      { return *semanticType; }

      /*!\return Returns the semantic type of the parameter (const).
       */
      const Expr &getSemanticType() const noexcept
      { return *semanticType; }

      /*!\return Returns optional guard.
       * \see hasGuard
       */
      Expr &getGuard() noexcept
      { return *guard; }

      /*!\return Returns optional guard (const).
       * \see hasGuardResult
       */
      const Expr &getGuard() const noexcept
      { return *guard; }

      /*!\return Returns optional guard result. Can be null (optional).
       */
      Expr &getGuardResult() noexcept
      { return *guardResult; }

      /*!\return Returns optional guard result (const). Can be null (optional).
       */
      const Expr &getGuardResult() const noexcept
      { return *guardResult; }

      bool hasGuard() const noexcept
      { return (bool) guard; }

      bool hasGuardResult() const noexcept
      { return (bool) guardResult; }
    };

    /*!\brief Function expression.
     */
    class FuncExpr : public IdExpr {
      std::unique_ptr<Expr> returnType; //!< Can be null (optional).
      std::vector<std::unique_ptr<Expr>> params;
      std::unique_ptr<TemplateExpr> templ;

      std::unique_ptr<Program> program;
    public:
      FuncExpr(const lexer::Position &pos, const std::string &name,
          std::unique_ptr<TemplateExpr> templ,
          std::unique_ptr<Expr> returnType,
          std::vector<std::unique_ptr<Expr>> params,
          std::unique_ptr<Program> program) noexcept;
      virtual ~FuncExpr();

      /*!\return Returns return type of the function. Can be null (optional).
       * \see hasReturnType
       */
      Expr &getReturnType() noexcept
      { return *returnType; }

      /*!\return Returns return type of the function (const).
       * \see hasReturnType
       */
      const Expr &getReturnType() const noexcept
      { return *returnType; }

      bool hasReturnType() const noexcept
      { return (bool) returnType; }

      /*!\return Returns parameters of function.
       */
      auto &getParameters() noexcept
      { return params; }

      /*!\return Returns parameters of function (const).
       */
      const auto &getParameters() const noexcept
      { return params; }

      /*!\return Returns function body. Can be null (optional).
       * \see isDeclared isDefined
       */
      Program &getProgram() noexcept
      { return *program; }

      /*!\return Returns function body (const). Can be null (optional).
       * \see isDeclared isDefined
       */
      const Program &getProgram() const noexcept
      { return *program; }

      /*!\return Returns true, if only declared, otherwise (defined) false.
       */
      bool isDeclared() const noexcept
      { return !program; } // If no program => Declared

      /*!\return Returns !isDeclared().
       * \see isDeclared
       */
      bool isDefined() const noexcept
      { return !isDeclared(); }
    };

    /*!\brief Class expression.
     */
    class ClassExpr : public IdExpr {
      std::vector<std::unique_ptr<Expr>> attributes;
      std::vector<std::unique_ptr<FuncExpr>> functions;
      std::unique_ptr<TemplateExpr> templ;
    public:
      ClassExpr(const lexer::Position &pos, const std::string &name,
          std::unique_ptr<TemplateExpr> templ,
          std::vector<std::unique_ptr<Expr>> attributes,
          std::vector<std::unique_ptr<FuncExpr>> functions) noexcept;
      virtual ~ClassExpr();

      /*!\return Returns (potential) attributes of the class.
       */
      auto &getAttributes() noexcept
      { return attributes; }

      /*!\return Returns (potential) attributes of the class (const).
       */
      const auto &getAttributes() const noexcept
      { return attributes; }
    };

    /*!\brief Enum expression.
     */
    class EnumExpr : public IdExpr {
      std::vector<std::unique_ptr<BiOpExpr>> constructors;
      std::unique_ptr<TemplateExpr> templ;
    public:
      EnumExpr(const lexer::Position &pos, const std::string &name,
          std::unique_ptr<TemplateExpr> templ,
          std::vector<std::unique_ptr<BiOpExpr>> constructors) noexcept;
      virtual ~EnumExpr();

      /*!\return Returns constructors of the enum.
       */
      auto &getConstructors() noexcept
      { return constructors; }

      /*!\return Returns constructors of the enum (const).
       */
      const auto &getConstructors() const noexcept
      { return constructors; }
    };

    /*!\brief Trait expression
     */
    class TraitExpr : public IdExpr {
      std::vector<std::unique_ptr<FuncExpr>> functions;
      std::unique_ptr<TemplateExpr> templ;
    public:
      /*!\brief Initialize instance of TraitExpr.
       * \param name Name of the trait.
       * \param functions Declared (not defined) functions.
       */
      TraitExpr(const lexer::Position &pos, const std::string &name,
          std::unique_ptr<TemplateExpr> templ,
          std::vector<std::unique_ptr<FuncExpr>> functions) noexcept;
      virtual ~TraitExpr();

      /*!\return Returns declared functions.
       */
      auto &getFunctions() noexcept
      { return functions; }

      /*!\return Returns declared functions (const).
       */
      const auto &getFunctions() const noexcept
      { return functions; }
    };

    /*!\brief Namespace expression.
     */
    class NmspExpr : public IdExpr {
      std::unique_ptr<Program> program;
    public:
      NmspExpr(const lexer::Position &pos, const std::string &name,
          std::unique_ptr<Program> program) noexcept;
      virtual ~NmspExpr();

      /*!\return Returns program of the namespace.
       */
      auto &getProgram() noexcept
      { return program; }

      /*!\return Returns program of the namespace.
       */
      const auto &getProgram() const noexcept
      { return program; }
    };

    /*!\brief Binary operator expression.
     */
    class BiOpExpr : public Expr {
      lexer::OperatorType opType;
      std::unique_ptr<Expr> lhs, rhs;
    public:
      BiOpExpr(const lexer::Position &pos,
          lexer::OperatorType opType,
          std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs) noexcept;
      virtual ~BiOpExpr();

      /*!\return Returns binary operator type.
       */
      lexer::OperatorType getOperator() const noexcept
      { return opType; }

      /*!\return Returns left-hand-side.
       */
      Expr &getLHS() noexcept
      { return *lhs; }
      
      /*!\return Returns left-hand-side (const).
       */
      const Expr &getLHS() const noexcept
      { return *lhs; }

      /*!\return Returns right-hand-side.
       */
      Expr &getRHS() noexcept
      { return *rhs; }

      /*!\return Returns right-hand-side (const).
       */
      const Expr &getRHS() const noexcept
      { return *rhs; }
    };

    /*!\brief Unary operator expression
     */
    class UnOpExpr : public Expr {
      lexer::OperatorType opType;
      lexer::OperatorPosition opPos;
      std::unique_ptr<Expr> expr;
    public:
      UnOpExpr(const lexer::Position &pos,
          lexer::OperatorPosition opPos, lexer::OperatorType opType,
          std::unique_ptr<Expr> expr) noexcept;
      virtual ~UnOpExpr();

      /*!\return Returns unary operator type.
       */
      lexer::OperatorType getOperator() const noexcept
      { return opType; }

      /*!\return Returns either op_lunary or op_runary.
       */
      lexer::OperatorPosition getOperatorPosition() const noexcept
      { return opPos; }

      /*!\return Returns expression associated with unary operator.
       */
      Expr &getExpression() noexcept
      { return *expr; }

      /*!\return Returns expression associated with unary operator (const).
       */
      const Expr &getExpression() const noexcept
      { return *expr; }
    };

    /*!\brief Brace expression
     */
    class BraceExpr : public Expr {
      std::unique_ptr<Expr> expr;
    public:
      BraceExpr(std::unique_ptr<Expr> expr) noexcept;
      virtual ~BraceExpr();

      Expr &getExpression() noexcept
      { return *expr; }

      const Expr &getExpression() const noexcept
      { return *expr; }
    };

    /*!\brief Template expression
     */
    class TemplateExpr : public Expr {
      std::vector<std::unique_ptr<Expr>> templates;
    public:
      TemplateExpr(std::vector<std::unique_ptr<Expr>> templates) noexcept;
      virtual ~TemplateExpr();

      std::vector<std::unique_ptr<Expr>> &getTemplates() noexcept;
      const std::vector<std::unique_ptr<Expr>> &getTemplates() const noexcept;
    };
  } // end namespace syntax
} // end namespace feder

#endif /* FEDER_SYNTAX_HPP */
