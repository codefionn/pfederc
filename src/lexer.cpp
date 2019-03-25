#include "feder/lexer.hpp"
using namespace feder::lexer;

bool feder::lexer::isLeftAssociative(OperatorType op) noexcept {
  return !isRightAssociative(op);
}

bool feder::lexer::isRightAssociative(OperatorType op) noexcept {
  switch (op) {
  case op_def:
  case op_asg:
  case op_asg_band:
  case op_asg_bxor:
  case op_asg_bor:
  case op_asg_add:
  case op_asg_sub:
  case op_asg_mul:
  case op_asg_div:
  case op_asg_mod:
  case op_decl:
    return true;
  default:
    return false;
  }
}

bool feder::lexer::isValidOperatorPosition(OperatorType op,
                                           OperatorPosition pos) noexcept {
  switch (pos) {
  case op_lunary:
    switch (op) {
    case op_dec:
    case op_inc:
    case op_add:
    case op_sub:
    case op_mul:
    case op_land:
    case op_bnot:
    case op_lnot:
      return true;
    default:
      return false;
    }
  case op_runary:
    switch (op) {
    case op_dec:
    case op_inc:
      return true;
    default:
      return false;
    }
  case op_binary:
    switch (op) {
    case op_dec:
    case op_inc:
    case op_bnot:
      return false;
    default:
      return true;
    }
  }

  return false;
}

bool feder::lexer::isPrimaryToken(TokenType tok) noexcept {
  switch (tok) {
  case tok_include:
  case tok_import:
  case tok_cbrace:
  case tok_cbrace_array:
  case tok_cbrace_template:
  case tok_delim:
  case tok_cmd:
    return false;
  default:
    return true;
  }
}

static std::size_t _getPrecedenceLeftUnary(OperatorType op) noexcept {
  switch (op) {
  case op_dec:
  case op_inc:
  case op_add:
  case op_sub:
  case op_mul:
  case op_lnot:
  case op_bnot:
  case op_mem:
    return 15;
  default:
    return 0;
  }
}

static std::size_t _getPrecedenceRightUnary(OperatorType op) noexcept {
  switch (op) {
  case op_dec:
  case op_inc:
  case op_fncall:
  case op_indexcall:
  case op_templatecall:
    return 16;
  default:
    return 0;
  }
}

static std::size_t _getPrecedenceBinary(OperatorType op) noexcept {
  switch (op) {
  case op_comma:
    return 1;
  case op_def:
    return 2;
  case op_asg:
  case op_asg_band:
  case op_asg_bxor:
  case op_asg_bor:
  case op_asg_add:
  case op_asg_sub:
  case op_asg_mul:
  case op_asg_div:
  case op_asg_mod:
  case op_asg_lsh:
  case op_asg_rsh:
  case op_impl:
    return 3;
  case op_lor:
    return 4;
  case op_land:
    return 5;
  case op_bor:
    return 6;
  case op_bxor:
    return 7;
  case op_band:
    return 8;
  case op_eq:
  case op_neq:
  case op_veq:
    return 9;
  case op_leq:
  case op_lt:
  case op_geq:
  case op_gt:
    return 10;
  case op_lsh:
  case op_rsh:
    return 11;
  case op_add:
  case op_sub:
    return 12;
  case op_mul:
  case op_div:
    return 13;
  case op_tcast:
  case op_tcheck:
  case op_decl:
    return 14;
  case op_mem:
  case op_deref_mem:
    return 17;
  default:
    // Sometimes this is necessary (when operator is not binary but only unary)
    return _getPrecedenceRightUnary(op);
  }
}

std::size_t feder::lexer::getPrecedence(OperatorType op,
                                        OperatorPosition pos) noexcept {
  switch (pos) {
  case op_binary:
    return _getPrecedenceBinary(op);
  case op_lunary:
    return _getPrecedenceLeftUnary(op);
  case op_runary:
    return _getPrecedenceRightUnary(op);
  default:
    return 0;
  }
}

// Position

Position::Position(Lexer *lexer, size_t columnStart, size_t columnEnd,
                   size_t lineStart, size_t lineEnd) noexcept
    : lexer{lexer}, columnStart{columnStart}, columnEnd{columnEnd},
      lineStart{lineStart}, lineEnd{lineEnd} {
#ifdef SANITY
  FEDER_SANITY_CHECK((bool)lexer, "lexer == nullptr");
  FEDER_SANITY_CHECK(columnStart <= columnEnd, "columnStart > columnEnd");
  FEDER_SANITY_CHECK(lineStart <= lineEnd, "lineStart > lineEnd");
#endif /* SANITY */
}

Position::Position(Lexer *lexer, size_t columnStart, size_t columnEnd,
                   size_t line) noexcept
    : lexer{lexer}, columnStart{columnStart}, columnEnd{columnEnd},
      lineStart{line}, lineEnd{line} {
#ifdef SANITY
  FEDER_SANITY_CHECK((bool)lexer, "lexer == nullptr");
  FEDER_SANITY_CHECK(columnStart <= columnEnd, "columnStart > columnEnd");
  FEDER_SANITY_CHECK(lineStart <= lineEnd, "lineStart > lineEnd");
#endif /* SANITY */
}

Position::Position(const Position &pos0, const Position &pos1) noexcept {
  if (pos0.getLexer() == pos1.getLexer()) {
    lexer = const_cast<Lexer *>(pos0.getLexer());
    columnStart = std::min(pos0.getColumnStart(), pos1.getColumnStart());
    columnEnd = std::max(pos0.getColumnEnd(), pos1.getColumnEnd());
    lineStart = std::min(pos0.getLineStart(), pos1.getLineStart());
    lineEnd = std::max(pos0.getLineEnd(), pos1.getLineEnd());
  } else {
    lexer = const_cast<Lexer *>(pos0.getLexer());
    columnStart = pos0.getColumnStart();
    columnEnd = pos0.getColumnEnd();
    lineStart = pos0.getLineStart();
    lineEnd = pos0.getLineEnd();
  }

#ifdef SANITY
  FEDER_SANITY_CHECK((bool)lexer, "lexer == nullptr");
  FEDER_SANITY_CHECK(columnStart <= columnEnd, "columnStart > columnEnd");
  FEDER_SANITY_CHECK(lineStart <= lineEnd, "lineStart > lineEnd");
#endif /* SANITY */
}

Position::Position(const Position &pos) noexcept
    : lexer{const_cast<Position &>(pos).getLexer()},
      columnStart{pos.getColumnStart()}, columnEnd{pos.getColumnEnd()},
      lineStart{pos.getLineStart()}, lineEnd{pos.getLineEnd()} {
#ifdef SANITY
  FEDER_SANITY_CHECK((bool)lexer, "lexer == nullptr");
  FEDER_SANITY_CHECK(columnStart <= columnEnd, "columnStart > columnEnd");
  FEDER_SANITY_CHECK(lineStart <= lineEnd, "lineStart > lineEnd");
#endif /* SANITY */
}

Position::~Position() {}

Position Position::minColumn() const noexcept {
  std::size_t newColumnEnd = columnEnd == 0 ? columnEnd : columnEnd - 1;
  std::size_t newColumnStart =
      newColumnEnd > columnStart ? columnStart : newColumnEnd;
  return Position(const_cast<Lexer *>(lexer), newColumnStart, newColumnEnd,
                  lineStart, lineEnd);
}

// Token

Token::Token(const Position &pos, TokenType tokenType) noexcept
    : pos(pos), tokenType{tokenType} {}

Token::Token(const Position &pos, NumberType numType,
             NumberValue numVal) noexcept
    : pos(pos), tokenType{tok_num}, numType{numType}, numVal{numVal} {}

Token::Token(const Position &pos, TokenType type,
             const std::string &str) noexcept
    : pos(pos), tokenType{type}, str(str) {}

Token::Token(const Position &pos, OperatorType opType) noexcept
    : pos(pos), tokenType{tok_op}, opType{opType} {}

Token::Token(const Position &pos, char c) noexcept
    : pos(pos), tokenType(tok_char), charVal{c} {}

Token::Token(const Token &tok) noexcept
    : pos(tok.getPosition()), tokenType{tok.getType()} {
  switch (tokenType) {
  case tok_op:
    opType = tok.getOperator();
    break;
  case tok_num:
    numType = tok.getNumberType();
    numVal = tok.getNumberValue();
    break;
  case tok_str:
  case tok_id:
  case tok_cmd:
    str = tok.getString();
  case tok_char:
    charVal = tok.getCharacter();
  default:
    break; // do nothing
  }
}

Token::~Token() {}

const Position &Token::getPosition() const noexcept { return pos; }

const std::string &Token::getString() const noexcept { return str; }

char Token::getCharacter() const noexcept { return charVal; }

NumberType Token::getNumberType() const noexcept { return numType; }

NumberValue Token::getNumberValue() const noexcept { return numVal; }

OperatorType Token::getOperator() const noexcept { return opType; }

TokenType Token::getType() const noexcept { return tokenType; }

bool Token::isRightAssociative() const noexcept {
  switch (getType()) {
  case tok_op:
    return lexer::isRightAssociative(getOperator());
  default:
    return false;
  }
}

// Lexer

Lexer::Lexer(const std::string &name, std::istream &input)
    : name(name), input{input}, lastlinechar{EOF - 2},
      curchar{EOF - 2}, // -3 makes nextToken read a new char first
      lineEnd{0}, columnEnd{0}, curtokval{nullptr}, skipNewLine{false} {}

Lexer::~Lexer() {
  if (curtokval)
    delete curtokval;
}

/*!\return Returns true if ' ', horizontal tab or vertical tab.
 * \param c
 */
static bool _iswhitespace(int c) {
  switch (c) {
  case ' ':
  case '\t':
  case '\v':
    return true;
  default:
    return false;
  }
}

int Lexer::nextChar() noexcept {
  if (!input)
    return EOF; // Invalid input stream

  if (curchar == EOF - 1) {
    columnEnd = 0;
    ++lineEnd;
  }

  int result = input.get();
  switch (result) {
  case EOF:
    ++columnEnd;

    curchar = EOF;
    lines.push_back(currentLine); // Add to line list

    currentLine = ""; // Reset current line
    break;
  case '\n':
  case '\r':
    if (lastlinechar != EOF - 1 && lastlinechar != EOF - 2 &&
        lastlinechar != result) {  // Extended NewLine character ?
      lastlinechar = EOF - 1;      // Only two consecutive characters accepted
      return curchar = nextChar(); // Only extension, return next readable char
    }

    lastlinechar = result;
    ++columnEnd;

    lines.push_back(currentLine); // Add to line list
    currentLine = "";             // Reset current line

    curchar = '\n';

    break;
  case '\t':
    columnEnd += 4;
    currentLine += "  ";
    curchar = result;
    break;
  default:
    ++columnEnd;
    currentLine += (char)result;
    curchar = result;
    break;
  }

  return curchar;
}

Position Lexer::getPosition() const noexcept {
  return Position(const_cast<Lexer *>(this), columnStart, columnEnd, lineStart,
                  lineEnd);
}

Position Lexer::getCursorPosition() const noexcept {
  return Position(const_cast<Lexer *>(this), columnEnd, columnEnd, lineEnd,
                  lineEnd);
}

void Lexer::readLine() noexcept {
  while (nextChar() != '\n' && currentChar() != EOF - 1 && currentChar() != EOF)
    ;

  skipNewLine = false;

  if (currentChar() != EOF)
    curchar = EOF - 1; // Set to read char state
}

/*!\return tok_num or tok_err.
 * \param lexer
 * \param curtok
 * \param numVal
 * \param numType
 */
static TokenType tokenNumber(Lexer &lexer, TokenType &curtok,
                             NumberValue &numVal,
                             NumberType &numType) noexcept {
  bool _isdecimal = true;
  std::uint64_t result = 0;
  std::string numStr;

  // dec, hex, bin, oct number
  if (lexer.currentChar() == '0') {
    lexer.nextChar(); // eat 0
    // hex, oct, bin or dec 0
    if (lexer.currentChar() == 'x') {
      _isdecimal = false;

      // hexadecimal
      lexer.nextChar(); // eat x

      if (!isxdigit(lexer.currentChar()))
        return curtok =
                   lexer.reportLexerError("Expected hexadecimal character!",
                                          lexer.getCursorPosition());

      while (isxdigit(lexer.currentChar())) {
        result *= 16;

        if (lexer.currentChar() >= '0' && lexer.currentChar() <= '9')
          result += lexer.currentChar() - '0';
        if (lexer.currentChar() >= 'A' && lexer.currentChar() <= 'F')
          result += lexer.currentChar() - 'A' + 10;
        if (lexer.currentChar() >= 'a' && lexer.currentChar() <= 'f')
          result += lexer.currentChar() - 'a' + 10;

        lexer.nextChar();
      }
    } else if (lexer.currentChar() == 'o') {
      _isdecimal = false;

      // octal
      lexer.nextChar(); // eat o
    } else if (lexer.currentChar() == 'b') {
      _isdecimal = false;

      // binary
      lexer.nextChar(); // eat b
    } else if (isdigit(lexer.currentChar())) {
      // invalid 0[num]
      // Current pos points to invalid character
      // but pointing to the zero is better (so column - 1)
      return curtok = lexer.reportLexerError(
                 "Number sequences can't be leaded by 0.",
                 lexer.getCursorPosition().minColumn());
    }

    // Or just 0
  } else {
    // dec
    while (isdigit(lexer.currentChar())) {
      numStr += lexer.currentChar();
      result *= 10;
      result += lexer.currentChar() - '0';

      lexer.nextChar(); // eat digit
    }
  }

  if (_isdecimal && lexer.currentChar() == '.') {
    // floating-point
    lexer.nextChar(); // eat .
    numStr += ".";

    while (isdigit(lexer.currentChar())) {
      numStr += lexer.currentChar();
      lexer.nextChar(); // eat number
    }

    switch (lexer.currentChar()) {
    case 'f':           // single-precision floating-point
      lexer.nextChar(); // eat f
      numType = num_f32;
      numVal.f32 = strtof(numStr.c_str(), nullptr);
      break;
    case 'F':           // Double-precision floating-point
      lexer.nextChar(); // eat F
    default:            // At default: Double-precision floating-point
      numType = num_f64;
      numVal.f64 = strtod(numStr.c_str(), nullptr);
      break;
    }

    if (isalpha(lexer.currentChar()) || lexer.currentChar() == '_')
      return curtok = lexer.reportLexerError(
                 std::string("Invalid character '") +
                 (char)lexer.currentChar() +
                 std::string("' directly after number token."));

    return curtok = tok_num;
  }
  // Number type given ?

  bool _isunsigned = false;
  if (lexer.currentChar() == 'u') {
    lexer.nextChar(); // eat 'u'
    _isunsigned = true;
  }

  switch (lexer.currentChar()) {
  case 's':           // 8-bit
    lexer.nextChar(); // eat s
    if (_isunsigned) {
      numType = num_u8;
      numVal.u8 = result;
    } else {
      numType = num_i8;
      numVal.i8 = result;
    }

    break;
  case 'S':           // 16-bit
    lexer.nextChar(); // eat S
    if (_isunsigned) {
      numType = num_u16;
      numVal.u16 = result;
    } else {
      numType = num_i16;
      numVal.i16 = result;
    }

    break;
  case 'L':           // 64-bit
    lexer.nextChar(); // eat s
    if (_isunsigned) {
      numType = num_u64;
      numVal.u64 = result;
    } else {
      numType = num_i64;
      numVal.i64 = result;
    }

    break;
  case 'l':           // 32-bit
    lexer.nextChar(); // eat l
  default:            // default 32-bit
    if (_isunsigned) {
      numType = num_u32;
      numVal.u32 = result;
    } else {
      numType = num_i32;
      numVal.i32 = result;
    }

    break;
  }

  if (isalpha(lexer.currentChar()) || lexer.currentChar() == '_')
    return curtok = lexer.reportLexerError(
               std::string("Invalid character '") + (char)lexer.currentChar() +
               std::string("' directly after number token."));

  return curtok = tok_num;
}

static TokenType tokenIdentifier(Lexer &lexer, TokenType &curtok,
                                 OperatorType &curop,
                                 std::string &str) noexcept {
  str = ""; // Reset string

  while (isalpha(lexer.currentChar()) || lexer.currentChar() == '_' ||
         (str != "_" && isdigit(lexer.currentChar()))) {
    str += lexer.currentChar();
    lexer.nextChar(); // eat char
  }

  if (str == "class")
    return curtok = tok_class;
  if (str == "namespace")
    return curtok = tok_nmsp;
  if (str == "trait")
    return curtok = tok_trait;
  if (str == "enum")
    return curtok = tok_enum;
  if (str == "func")
    return curtok = tok_func;
  if (str == "Func")
    return curtok = tok_vfunc;
  if (str == "match")
    return curtok = tok_match;

  if (str == "safe") {
    curop = op_safe;
    return curtok = tok_op;
  }

  return curtok = tok_id;
}

/*!\return Returns true, if valid escape sequence.
 * False if invalid.
 * \param str resolved sequence from escape sequence second character.
 */
static bool escapeSequence(Lexer &lexer, std::string &str) {
  switch (lexer.currentChar()) {
  case '0':
    str += '\0';
    break;
  case '\'':
    str += '\'';
    break;
  case '\"':
    str += '\"';
    break;
  case '\\':
    str += '\\';
    break;
  case 'a':
    str += '\a';
    break;
  case 'b':
    str += '\b';
    break;
  case 'f':
    str += '\f';
    break;
  case 'n':
    str += '\n';
    break;
  case 'r':
    str += '\r';
    break;
  case 't':
    str += '\t';
    break;
  case 'v':
    str += '\v';
    break;
  default:
    Position pos = lexer.getPosition();
    lexer.reportLexerError("Invalid escape sequence.", pos);
    return false;
  }

  return true;
}

static TokenType tokenString(Lexer &lexer, TokenType &curtok,
                             std::string &str) noexcept {
  str = "";         // Reset string
  lexer.nextChar(); // eat "

  while (lexer.currentChar() != '\"' && lexer.currentChar() != EOF) {
    if (lexer.currentChar() == EOF - 1 || lexer.currentChar() == '\n') {
      lexer.nextChar(); // eat newline character

      // Skip spaces
      while (_iswhitespace(lexer.currentChar()))
        lexer.nextChar();
      continue;
    } else if (lexer.currentChar() == '\\') {
      lexer.nextChar(); // eat '\'
      if (!escapeSequence(lexer, str))
        return curtok = tok_err;
    } else {
      str += lexer.currentChar();
    }

    lexer.nextChar(); // eat char
  }

  if (lexer.currentChar() != '\"')
    return curtok = lexer.reportLexerError("Expected \'\"\' not end-of-file.");

  lexer.nextChar(); // eat "

  return curtok = tok_str;
}

static TokenType tokenChar(Lexer &lexer, TokenType &curtok,
                           std::string &str) noexcept {
  str = "";         // Reset string
  lexer.nextChar(); // eat '

  if (lexer.currentChar() == '\\') { // escape sequence
    lexer.nextChar();                // eat '\\'
    if (!escapeSequence(lexer, str))
      return curtok = tok_err;

    lexer.nextChar(); // eat escaped character
  } else if (isprint(lexer.currentChar())) {
    str += lexer.currentChar();
    lexer.nextChar(); // eat char
  } else {
    return curtok = lexer.reportLexerError("Invalid not-printable character.");
  }

  if (lexer.currentChar() != '\'')
    return curtok = lexer.reportLexerError("Expected '.");

  lexer.nextChar(); // eat '

  return curtok = tok_char;
}

TokenType Lexer::constructToken() noexcept {
  if (curchar == EOF - 1 || curchar == EOF - 2)
    nextChar();

  while (_iswhitespace(curchar))
    nextChar(); // Ignore space

  lineStart = lineEnd;
  columnStart = columnEnd;

  // Static tokens
  switch (curchar) {
  case EOF:
    return curtok = tok_eof;
  case '\n':
    curchar = EOF - 1;
    return curtok = tok_eol;

  case ';':
    nextChar(); // eat ;
    return curtok = tok_delim;

  case '~':
    nextChar(); // eat ~
    curop = op_bnot;
    return curtok = tok_op;
  case '.':
    nextChar(); // eat .
    curop = op_mem;
    return curtok = tok_op;
  case ',':
    nextChar(); // eat ,
    curop = op_comma;
    return curtok = tok_op;
  case '!':
    nextChar(); // eat !
    if (curchar == '=') {
      nextChar(); // eat =
      curop = op_neq;
    } else
      curop = op_lnot;

    return curtok = tok_op;
  case ':':
    nextChar(); // eat :
    switch (curchar) {
    case '=':
      nextChar(); // eat =
      curop = op_def;
      break;
    case ':':
      nextChar(); // eat :
      curop = op_tcast;
      break;
    case '?':
      nextChar(); // eat ?
      curop = op_tcheck;
      break;
    default:
      curop = op_decl;
      break;
    }

    return curtok = tok_op;
  case '=':
    nextChar(); // eat =
    if (curchar == '=') {
      nextChar(); // eat =
      if (curchar == '=') {
        nextChar(); // eat =
        curop = op_veq;
      } else
        curop = op_eq;
    } else
      curop = op_asg;

    return curtok = tok_op;
  case '&':
    nextChar(); // eat &
    switch (curchar) {
    case '=':
      nextChar(); // eat =
      curop = op_asg_band;
      break;
    case '&':
      nextChar(); // eat &
      curop = op_land;
      break;
    default:
      curop = op_band;
      break;
    }

    return curtok = tok_op;
  case '^':
    nextChar(); // eat ^
    if (curchar == '=') {
      nextChar(); // eat =
      curop = op_asg_bxor;
    } else
      curop = op_bxor;

    return curtok = tok_op;
  case '|':
    nextChar(); // eat |
    switch (curchar) {
    case '|':
      nextChar(); // eat |
      curop = op_land;
      break;
    case '=':
      nextChar(); // eat =
      curop = op_asg_band;
      break;
    default:
      curop = op_band;
      break;
    }

    return curtok = tok_op;
  case '+':
    nextChar(); // eat +
    switch (curchar) {
    case '+':
      nextChar(); // eat +
      curop = op_inc;
      break;
    case '=':
      nextChar(); // eat =
      curop = op_asg_add;
      break;
    default:
      curop = op_add;
      break;
    }

    return curtok = tok_op;
  case '-':
    nextChar(); // eat -
    switch (curchar) {
    case '-':
      nextChar(); // eat -
      curop = op_dec;
      break;
    case '=':
      nextChar(); // eat =
      curop = op_asg_sub;
      break;
    case '>':
      nextChar(); // eat >
      curop = op_deref_mem;
      break;
    default:
      curop = op_sub;
      break;
    }

    return curtok = tok_op;
  case '*':
    nextChar(); // eat *
    if (curchar == '=') {
      nextChar(); // eat =
      curop = op_asg_mul;
    } else
      curop = op_mul;

    return curtok = tok_op;
  case '/':
    nextChar(); // eat /
    if (curchar == '=') {
      nextChar(); // eat =
      curop = op_asg_div;
    } else if (curchar == '/') {
      nextChar(); // eat /
      readLine(); // read till eof/eol
      if (curchar == EOF)
        return curtok = tok_eof;
      // return newline
      curchar = EOF - 1;
      return curtok = tok_eol;
    } else if (curchar == '*') {
      nextChar(); // eat *
      while (curchar != EOF) {
        if (curchar == '*') {
          nextChar(); // eat *
          if (curchar == '/')
            break;
        } else
          nextChar(); // eat char

        if (curchar == '\n')
          curchar = EOF - 2;
      }

      if (curchar != '/')
        return curtok = reportLexerError("Expected '*/'.");
      nextChar();                       // eat '/'
      return curtok = constructToken(); // Return next token
    } else
      curop = op_div;

    return curtok = tok_op;
  case '%':
    nextChar(); // eat %
    if (curchar == '=') {
      nextChar(); // eat =
      curop = op_asg_mod;
    } else
      curop = op_mod;

    return curtok = tok_op;
  case '<':
    nextChar(); // eat <
    switch (curchar) {
    case '=':
      nextChar(); // eat =
      curop = op_leq;
      break;
    case '<':
      nextChar(); // eat <
      curop = op_lsh;
      break;
    default:
      curop = op_lt;
      break;
    }

    return curtok = tok_op;
  case '>':
    nextChar(); // eat >
    switch (curchar) {
    case '=':
      nextChar(); // eat =
      curop = op_geq;
      break;
    case '>':
      nextChar(); // eat >
      curop = op_rsh;
      break;
    default:
      curop = op_gt;
      break;
    }

    return curtok = tok_op;
  case '(':
    nextChar(); // eat (
    return curtok = tok_obrace;
  case ')':
    nextChar(); // eat )
    return curtok = tok_cbrace;
  case '[':
    nextChar(); // eat [
    return curtok = tok_obrace_array;
  case ']':
    nextChar(); // eat ]
    return curtok = tok_cbrace_array;
  case '{':
    nextChar(); // eat {
    return curtok = tok_obrace_template;
  case '}':
    nextChar(); // eat }
    return curtok = tok_cbrace_template;
  default:
    break; // Do nothing
  }

  // Dynamic tokens

  if (isdigit(curchar)) {
    return tokenNumber(*this, curtok, curnumval, curnumtype);
  }

  if (isalpha(curchar) || curchar == '_') {
    // identifier
    return tokenIdentifier(*this, curtok, curop, curstr);
  }

  if (curchar == '\"') {
    return tokenString(*this, curtok, curstr);
  }

  if (curchar == '\'') {
    return tokenChar(*this, curtok, curstr);
  }

  if (isprint(curchar))
    return curtok = reportLexerError(std::string("Invalid character \'") +
                                     (char)curchar + "\'.");
  else
    return curtok = reportLexerError("Invalid not-printable character.");
}

const Token &Lexer::nextToken() noexcept {
  // First come the pushed_tokens
  if (!pushed_tokens.empty()) {
    // queue
    auto last = pushed_tokens.end() - 1;
    curtokval = new Token(*last); // Copy token
    // remove last
    pushed_tokens.erase(last);

    return *curtokval;
  }

  do {
    constructToken(); // aquire next token
  } while (skipNewLine && curtok == lexer::tok_eol);

  if (curtokval)
    delete curtokval;

  switch (curtok) {
  case tok_id:
  case tok_str:
    return *(curtokval = new Token(getPosition(), curtok, curstr));
  case tok_char:
    return *(curtokval = new Token(getPosition(), curstr[0]));
  case tok_num:
    return *(curtokval = new Token(getPosition(), curnumtype, curnumval));
  case tok_op:
    return *(curtokval = new Token(getPosition(), curop));
  default:
    return *(curtokval = new Token(getPosition(), curtok));
  }
}

TokenType Lexer::reportLexerError(const std::string &msg) noexcept {
  return reportLexerError(msg, getPosition());
}

TokenType Lexer::reportLexerError(const std::string &msg,
                                  const Position &pos) noexcept {
  readLine(); // read till EOL

  size_t startindex = pos.getColumnStart() - 1;
  if (pos.getColumnStart() == 0)
    startindex = 0;
  size_t endindex = pos.getColumnEnd() - 1;
  if (pos.getColumnEnd() == 0)
    endindex = 0;

  if (startindex > endindex)
    std::swap(startindex, endindex);

  for (size_t i = pos.getLineStart(); i <= pos.getLineEnd(); ++i) {
    std::cerr << getLines()[i] << std::endl;
  }

  // Print position
  for (size_t i = 0; i <= endindex; ++i) {
    if (i < startindex)
      std::cerr << " ";
    else
      std::cerr << "^";
  }
  std::cerr << std::endl;

  if (!msg.empty())
    std::cerr << "error:" << name << ":" << pos.getLineStart() + 1 << ": "
              << msg << std::endl;

  return tok_err;
}

void Lexer::reportSyntaxError(const std::string &msg,
                              const Position &pos) noexcept {
  readLine(); // read till EOL

  reportSemanticError(msg, pos);
}

void Lexer::reportSemanticError(const std::string &msg,
                                const Position &pos) noexcept {
  size_t startindex = pos.getColumnStart() - 1;
  if (pos.getColumnStart() == 0)
    startindex = 0;
  size_t endindex = pos.getColumnEnd() - 2;
  if (pos.getColumnEnd() <= 1)
    endindex = 0;

  if (startindex > endindex)
    std::swap(startindex, endindex);

  for (size_t i = pos.getLineStart(); i <= pos.getLineEnd(); ++i) {
    std::cerr << getLines()[i] << std::endl;
  }

  // Print position
  for (size_t i = 0; i <= endindex; ++i) {
    if (i < startindex)
      std::cerr << " ";
    else
      std::cerr << "^";
  }
  std::cerr << std::endl;

  if (!msg.empty())
    std::cerr << "error:" << name << ":" << pos.getLineStart() + 1 << ": "
              << msg << std::endl;
}

std::size_t Token::getPrecedence(OperatorPosition pos) const noexcept {
  switch (getType()) {
  case tok_op:
    return feder::lexer::getPrecedence(getOperator(), pos);
  case tok_obrace:
  case tok_obrace_array:
  case tok_obrace_template:
    return 16;
  default:
    return 0; // Minimal precedence
  }
}

std::string std::to_string(feder::lexer::TokenType tok) {
  switch (tok) {
  case tok_class:
    return "class";
  case tok_enum:
    return "enum";
  case tok_trait:
    return "trait";
  case tok_func:
    return "func";
  case tok_vfunc:
    return "Func";
  case tok_nmsp:
    return "namespace";
  case tok_if:
    return "if";
  case tok_else:
    return "else";
  case tok_for:
    return "for";
  case tok_do:
    return "do";
  case tok_match:
    return "match";
  case tok_include:
    return "include";
  case tok_import:
    return "import";
  case tok_id:
    return "identifier";
  case tok_delim:
    return ";";
  case tok_str:
    return "string";
  case tok_char:
    return "character";
  case tok_num:
    return "number";
  case tok_obrace:
    return "(";
  case tok_cbrace:
    return ")";
  case tok_obrace_array:
    return "[";
  case tok_cbrace_array:
    return "]";
  case tok_obrace_template:
    return "{";
  case tok_cbrace_template:
    return "}";
  case tok_op:
    return "operator";
  case tok_cmd:
    return "command";
  case tok_eol:
    return "end-of-line";
  case tok_eof:
    return "end-of-file";
  case tok_err:
    return "error";
  default:
    feder::fatal("Unknown token type.");
    return "";
  }
}

std::string std::to_string(feder::lexer::OperatorType op) {
  switch (op) {
  case op_comma:
    return ",";
  case op_def:
    return ":=";
  case op_asg:
    return "=";
  case op_asg_band:
    return "&=";
  case op_asg_bxor:
    return "^=";
  case op_asg_bor:
    return "|=";
  case op_asg_add:
    return "+=";
  case op_asg_sub:
    return "-=";
  case op_asg_mul:
    return "*=";
  case op_asg_div:
    return "/=";
  case op_asg_mod:
    return "%=";
  case op_asg_lsh:
    return "<<=";
  case op_asg_rsh:
    return ">>=";
  case op_land:
    return "&&";
  case op_lor:
    return "||";
  case op_eq:
    return "==";
  case op_neq:
    return "!=";
  case op_veq:
    return "===";
  case op_leq:
    return "<=";
  case op_geq:
    return ">=";
  case op_lt:
    return "<";
  case op_gt:
    return ">";
  case op_band:
    return "&";
  case op_bxor:
    return "^";
  case op_bor:
    return "|";
  case op_lsh:
    return "<<";
  case op_rsh:
    return ">>";
  case op_add:
    return "+";
  case op_sub:
    return "-";
  case op_mul:
    return "*";
  case op_div:
    return "/";
  case op_mod:
    return "%";
  case op_decl:
    return ":";
  case op_tcast:
    return "::";
  case op_tcheck:
    return ":?";
  case op_lnot:
    return "!";
  case op_bnot:
    return "~";
  case op_inc:
    return "++";
  case op_dec:
    return "--";
  case op_safe:
    return "safe";
  case op_mem:
    return ".";
  case op_deref_mem:
    return "->";
  case op_fncall:
    return "()";
  case op_indexcall:
    return "[]";
  case op_templatecall:
    return "{}";
  default:
    feder::fatal("Unknown operator type.");
    return "";
  }
}
