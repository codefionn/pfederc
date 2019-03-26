#ifndef FEDER_LEXER_HPP
#define FEDER_LEXER_HPP

/*!\file feder/lexer.hpp
 * \brief Lexical analysis/Tokenizer.
 */

/*!\defgroup lexer Lexical analysis
 */

#include "feder/global.hpp"

namespace feder {
namespace lexer {
/*!\addtogroup lexer
 * \brief Semantics for lexical analysis/tokenizer.
 *
 * \{
 */

/*!\brief Token types.
 */
enum TokenType {
  tok_class, //!< 'class'
  tok_enum,  //!< 'enum'
  tok_trait, //!< 'trait'
  tok_func,  //!< 'func'
  tok_vfunc, //!< 'Func'
  tok_nmsp,  //!< 'namespace'

  tok_if,    //!< 'if'
  tok_else,  //!< 'else'
  tok_for,   //!< 'for'
  tok_do,    //!< 'do'
  tok_match, //!< 'match'

  tok_include, //!< 'include'
  tok_import,  //!< 'import'

  tok_id,   //!< Identifier
  tok_str,  //!< String
  tok_char, //!< Character

  tok_num, //!< Number

  tok_obrace,          //!< (
  tok_cbrace,          //!< )
  tok_obrace_array,    //!< [
  tok_cbrace_array,    //!< ]
  tok_obrace_template, //!< {
  tok_cbrace_template, //!< }

  tok_delim, //!< ;
  tok_return, //!< 'return'

  tok_op, //!< Operator

  tok_cmd, //!< #.*$

  tok_eol, //!< End-Of-Line
  tok_eof, //!< End-Of-File

  tok_err, //!< Token returned on error
};

/*!\brief Operator types.
 */
enum OperatorType {
  op_comma, //!< ,

  op_def, //!< :=
  op_asg, //!< =

  op_asg_band, //!< &=
  op_asg_bxor, //!< ^=
  op_asg_bor,  //!< |=
  op_asg_add,  //!< +=
  op_asg_sub,  //!< -=
  op_asg_mul,  //!< *=
  op_asg_div,  //!< /=
  op_asg_mod,  //!< %=
  op_asg_lsh,  //!< \<\<=
  op_asg_rsh,  //!< \<\<=

  op_land, //!< &&
  op_lor,  //!< ||

  op_eq,  //!< ==
  op_neq, //!< !=
  op_veq, //!< ===

  op_leq, //!< \<=
  op_geq, //!< \>=
  op_lt,  //!< \<
  op_gt,  //!< \>

  op_impl, //!< =\>

  op_band, //!< &
  op_bxor, //!< ^
  op_bor,  //!< |

  op_lsh, //!< \<\<
  op_rsh, //!< \>\>

  op_add, //!< +
  op_sub, //!< -
  op_mul, //!< *
  op_div, //!< /
  op_mod, //!< %

  op_decl,   //!< :
  op_tcast,  //!< ::
  op_tcheck, //!< :?

  op_lnot, //!< !
  op_bnot, //!< ~
  op_inc,  //!< ++
  op_dec,  //!< --

  op_safe,   //!< 'safe'

  op_mem,       //!< .
  op_deref_mem, //!< ->

  /* Special non-character sequence operators */

  op_fncall,       //!< a()
  op_indexcall,    //!< a[]
  op_templatecall, //!< a{T}
};

/*!\return Returns true, if the operator is left associative,
 * otherwise false.
 * \see isRightAssociative
 */
bool isLeftAssociative(OperatorType op) noexcept;

/*!\return Returns true, if the operator is right associative,
 * otherwise false.
 * \see isLeftAssociative
 */
bool isRightAssociative(OperatorType op) noexcept;

/*!\brief Operator positions.
 */
enum OperatorPosition {
  op_lunary, //!< Left-side unary
  op_runary, //!< Right-side unary
  op_binary, //!< Binary
};

/*!\return Returns true, if operator op can be used in pos.
 * \param op
 * \param pos
 */
bool isValidOperatorPosition(OperatorType op, OperatorPosition pos) noexcept;

/*!\return Returns precedence of the operator at pos. 0 is minimum
 * (least-binding) precedence. Returns 0 if invalid operator (at pos).
 * \param op
 * \param pos
 */
std::size_t getPrecedence(OperatorType op, OperatorPosition pos) noexcept;

/*!\return Returns true, if tok is a primary expression token. Otherwise,
 * false.
 * \param tok
 */
bool isPrimaryToken(TokenType tok) noexcept;

/*!\brief Numeric types.
 */
enum NumberType {
  num_i8,  //!< 8-bit integer
  num_i16, //!< 16-bit integer
  num_i32, //!< 32-bit integer
  num_i64, //!< 64-bit integer
  num_u8,  //!< unsigned 8-bit integer
  num_u16, //!< unsigned 16-bit integer
  num_u32, //!< unsigned 32-bit integer
  num_u64, //!< unsigned 64-bit integer
  num_f32, //!< single-precision floating-point
  num_f64, //!< double-precision floating-point
};

/*!\brief Union for different number values.
 */
union NumberValue {
  std::int8_t i8;    //!< \see NumberType::num_i8
  std::int16_t i16;  //!< \see NumberType::num_i16
  std::int32_t i32;  //!< \see NumberType::num_i32
  std::int64_t i64;  //!< \see NumberType::num_i64
  std::uint8_t u8;   //!< \see NumberType::num_u8
  std::uint16_t u16; //!< \see NumberType::num_u16
  std::uint32_t u32; //!< \see NumberType::num_u32
  std::uint64_t u64; //!< \see NumberType::num_u64
  float f32;         //!< \see NumberType::num_f32
  double f64;        //!< \see NumberType::num_f64
};

// Pre-declaration
class Position;
class Token;
class Lexer;

/*!\brief Class for referencing code position.
 * \see Lexer
 */
class Position {
  Lexer *lexer; //
  std::size_t columnStart, columnEnd, lineStart, lineEnd;

public:
  /*!\brief Initializes code position reference.
   * \param lexer Where the position is referenced from.
   * \param columnStart
   * \param columnEnd
   * \param lineStart
   * \param lineEnd
   */
  Position(Lexer *lexer, size_t columnStart, size_t columnEnd, size_t lineStart,
           size_t lineEnd) noexcept;

  /*!\brief Initializes code position reference.
   * \param lexer Where the position is referenced from.
   * \param columnStart
   * \param columnEnd
   * \param line
   */
  Position(Lexer *lexer, size_t columnStart, size_t columnEnd,
           size_t line) noexcept;

  Position() noexcept : lexer{nullptr} {}

  /*!\brief Copy-constructor.
   */
  Position(const Position &pos) noexcept;

  /*!\brief Merges position pos0 and pos1 (the resulting Position is the
   * maximal marked area between pos0 and pos1). If lexer of pos0 and pos1
   * is not the same, the new position will be a copy of pos0.
   * \param pos0
   * \param pos1
   */
  Position(const Position &pos0, const Position &pos1) noexcept;

  virtual ~Position();

  /*!\return Where to find the referenced position.
   */
  Lexer *getLexer() noexcept { return lexer; }

  /*!\return Where to find the referenced position (const).
   */
  const Lexer *getLexer() const noexcept { return lexer; }

  /*!\return Returns first column of the marked area.
   * \see getColumnEnd
   */
  std::size_t getColumnStart() const noexcept { return columnStart; }

  /*!\return Returns last column of the marked area.
   * \see getColumnStart
   */
  std::size_t getColumnEnd() const noexcept { return columnEnd; }

  /*!\return Returns first line of the marked area.
   * \see getLineEnd
   */
  std::size_t getLineStart() const noexcept { return lineStart; }

  /*!\return Returns last line of the marked area.
   * \see getLineStart
   */
  std::size_t getLineEnd() const noexcept { return lineEnd; }

  /*!\return Returns position where 1 is substracted from columnEnd.  If
   * columnStart is greater than columnEnd, 1 will also be substracted from
   * columnStart.
   */
  Position minColumn() const noexcept;
};

/*!\brief A Token is the result of the lexical analysis nextToken.
 * \see Lexer.nextToken
 */
class Token {
  Position pos;
  TokenType tokenType;

  std::string str;     //!< If token type is tok_id, tok_str or tok_cmd.
  OperatorType opType; //!< If token type is tok_op.
  NumberType numType;  //!< If token type is tok_num.
  NumberValue numVal;  //!< If token type is tok_num.
  char charVal;        //!< If token type is tok_char.
public:
  /*!\brief Initializes token.
   * \param pos Position in program.
   * \param tokenType Type of the token.
   */
  Token(const Position &pos, TokenType tokenType) noexcept;

  /*!\brief Initializes token with type tok_num.
   * \param pos
   * \param numType
   * \param numVal
   */
  Token(const Position &pos, NumberType numType, NumberValue numVal) noexcept;

  /*!\brief Initializes token with type tok_id or tok_str.
   * \param pos
   * \param type
   * \param str
   */
  Token(const Position &pos, TokenType type, const std::string &str) noexcept;

  /*!\brief Initializes token with type tok_op.
   * \param pos
   * \param opType
   */
  Token(const Position &pos, OperatorType opType) noexcept;

  /*!\brief Initializes token with type tok_char.
   * \param pos
   * \param c
   */
  Token(const Position &pos, char c) noexcept;

  /*!\brief Empty initialization. Any methods of the initialized must not
   * be used. Must be assigned to another valid Token first.
   *
   *     Token tok; // Only reassigning should be used (copy-constructor)
   *     tok = lexer.nextToken(); // Now tok can be used
   */
  Token() noexcept {}

  /*!\return Returns precedence of token.
   * \param opPos Position of the operator
   */
  std::size_t getPrecedence(OperatorPosition opPos = op_binary) const noexcept;

  /*!\brief Copy-constructor.
   * \param tok Token to copy.
   */
  Token(const Token &tok) noexcept;

  virtual ~Token();

  /*!\return Returns the token's code position.
   */
  const Position &getPosition() const noexcept;

  /*!\return Returns string. Must only be used if token type is tok_id,
   * tok_str or tok_cmd. Without escape sequences.
   */
  const std::string &getString() const noexcept;

  /*!\return Returns character. Must only be used if token type is
   * tok_char.
   */
  char getCharacter() const noexcept;

  /*!\return Returns number type. Token type must be tok_num.
   */
  NumberType getNumberType() const noexcept;

  /*!\return Returns number value. Token type must be tok_num.
   */
  NumberValue getNumberValue() const noexcept;

  /*!\return Returns operator type.
   */
  OperatorType getOperator() const noexcept;

  /*!\return Return type of token.
   */
  TokenType getType() const noexcept;

  /*!\return Returns if (operator) token is right-associative.
   */
  bool isRightAssociative() const noexcept;

  /*!\return Returns true, if getType() and type are equal, otherwise
   * false.
   */
  bool operator==(TokenType type) const noexcept { return getType() == type; }

  /*!\return Returns false, if getType() and type are equal, otherwise
   * true.
   */
  bool operator!=(TokenType type) const noexcept { return getType() != type; }

  /*!\return Returns true, if getType() is operator and operator
   * equals type, otherwise false is returned.
   */
  bool operator==(OperatorType optype) const noexcept {
    return getType() == lexer::tok_op && getOperator() == optype;
  }

  /*!\return Returns false, if getType() is not operator or operator
   * isn't type, otherwise true is returned.
   */
  bool operator!=(OperatorType type) const noexcept {
    return getType() != lexer::tok_op || getOperator() != type;
  }
};

/*!\brief Describing a lexer instance (e.g. a file).
 *
 * This class can be used to tokenize an input stream.
 *
 *     Lexer lexer("<stdin>", std::cin);
 *     Token tok;
 *     while ((tok = lexer.nextToken()).getType() != tok_eof) {
 *       // Do something with the token
 *     }
 */
class Lexer {
  std::string name;    //!< Name of the lexer (file-name)
  std::istream &input; //!< Input stream to read from.

  std::string currentLine;        //!< String holding current line.
  std::vector<std::string> lines; //!< All old lines, where 0 is the first.

  std::size_t lineStart,   //!< First line of the current token.
      lineEnd;             //!< Last line of the current token.
  std::size_t columnStart, //!< First position of the current token.
      columnEnd;           //!< Last position of the current token.

  int lastlinechar;   //!< Last new-line character (CR or LF).
  int curchar = EOF;  //!< Current character, -2 if next char should be called
  TokenType curtok;   //!< Current token type
  OperatorType curop; //!< Current operator type
  NumberType curnumtype; //!< Current number type.
  NumberValue curnumval; //!< Current number value.
  std::string curstr;    //!< Current string. Without escape sequences.

  std::vector<Token> pushed_tokens; //!< Next tokens (Stack).

  Token *curtokval;
  TokenType constructToken() noexcept;

public:
  /*!\brief Initialize the lexer.
   * \param name Name representive for the input stream.
   * \param input Input stream to read characters from.
   * \see getName
   */
  Lexer(const std::string &name, std::istream &input);
  Lexer(const Lexer &lexer) = delete;

  virtual ~Lexer();

  /*!\return Returns name of lexer (e.g. file name, just a name
   * representing the input stream).
   */
  const std::string &getName() const noexcept { return name; }

  /*!\return Returns all lines read with nextChar.
   * \see nextChar
   */
  auto &getLines() const noexcept { return lines; }

  /*!\return Returns next read character from input.
   *
   * If an EOL character is read it will be returned as '\n' regardless
   * wether it was '\n' or '\r'. Also if a the EOL character occurs, which
   * is different from the previous one, it will be ignored. The next EOL
   * character will not be ignored. Every not-ignored EOL character will
   * cause the currentLine to be added to lines (getLines) and increase
   * at the next call of nextChar, the lineCount by 1 and reset columnEnd
   * to 0 (getPosition().getColumnEnd()).
   *
   * If the input stream is bad, EOF will be returned.
   */
  int nextChar() noexcept;

  /*!\return Returns lastest result of nextChar.
   * \see nextChar
   */
  int currentChar() const noexcept { return curchar; }

  /*!\brief Reads line till end. Must only be used for error printing.
   *
   * After readLine was called, it is guaranteed, that currentChar() is
   * either EOF or EOF - 1. Also that currentLine is empty and the last
   * line was added to lines (getLines). Sets skipNewLine to false.
   */
  void readLine() noexcept;

  /*!\return Returns next token aquired. Tokenize input.
   *
   * Updates getPosition to the position of the returned token.
   */
  const Token &nextToken() noexcept;

  /*!\brief Pushes token on a stack. Call nextToken(), will return the
   * last element (and remove it).
   * \param tok
   */
  void pushToken(const Token &tok) noexcept;

  /*!\return Returns latest token returned by nextToken.
   */
  const Token &currentToken() const noexcept { return *curtokval; }

  /*!\return Returns position of latest token returned by nextToken.
   *
   * A black/control character is also regarded as a column. Known that
   * getColumnEnd of position is actually not the last column but the
   * column right after the last column. Also columns are counted starting
   * from 1, not 0 (the first column has the index 1). So computing the
   * real position of is done like this:
   *
   *     Position pos = lexer.getPosition();
   *     Position realPos(pos.getLexer(),
   *       pos.getColumnStart() - 1,
   *       pos.getColumnEnd() - 2,
   *       pos.getLineStart(), pos.getLineEnd());
   *
   * Note that any reportSyntaxError and reportSemanticError/Warning
   * expect it to be like this. So don't try to "correct" this "error".
   */
  Position getPosition() const noexcept;

  /*!\return Returns position of latest cursor position.
   *
   * A black/control character is also regarded as a column. Known that
   * getColumnEnd of position is actually not the last column but the
   * column right after the last column. Also columns are counted starting
   * from 1, not 0 (the first column has the index 1). So computing the
   * real position of is done like this:
   *
   *     Position pos = lexer.getPosition();
   *     Position realPos(pos.getLexer(),
   *       pos.getColumnStart() - 1,
   *       pos.getColumnEnd() - 2,
   *       pos.getLineStart(), pos.getLineEnd());
   *
   * Note that any reportSyntaxError and reportSemanticError/Warning
   * expect it to be like this. So don't try to "correct" this "error".
   */
  Position getCursorPosition() const noexcept;

  /*!\brief Reports a lexical error. Reads till EOL.
   * \param msg
   * \see getPosition readLine
   */
  TokenType reportLexerError(const std::string &msg) noexcept;

  /*!\brief Reports a lexical error. Reads till EOL.
   * \param msg
   * \param pos
   * \see getPosition readLine
   */
  TokenType reportLexerError(const std::string &msg,
                             const Position &pos) noexcept;

  /*!\brief Reports a syntax error. Reads till EOL.
   * \param msg
   * \param pos
   * \see getPosition readLine
   */
  void reportSyntaxError(const std::string &msg, const Position &pos) noexcept;

  /*!\brief Reports a semantic error.
   * \param msg
   * \param pos
   * \see getPosition readLine
   */
  void reportSemanticError(const std::string &msg,
                           const Position &pos) noexcept;

  /*!\brief Reports a semantic warning.
   * \param msg
   * \param pos
   * \see getPosition readLine
   */
  void reportSemanticWarning(const std::string &msg,
                             const Position &pos) noexcept;

  bool skipNewLine;
};

/*!\} */
} // end namespace lexer
} // end namespace feder

namespace std {
std::string to_string(feder::lexer::TokenType tok);
std::string to_string(feder::lexer::OperatorType tok);
} // namespace std

#endif /* FEDER_LEXER_HPP */
