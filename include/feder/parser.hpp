#ifndef FEDER_PARSER_HPP
#define FEDER_PARSER_HPP

/*!\file feder/parser.hpp
 * \brief Parser
 */

#include "feder/global.hpp"
#include "feder/lexer.hpp"
#include "feder/syntax.hpp"

/*!\defgroup parser Syntactic analysis/Parser
 */

namespace feder {
namespace parser {
/*!\addtogroup parser
 * \brief Syntactic analysis/Parser.
 *
 *\{
 */

/*!\brief Parse primary token.
 */
std::unique_ptr<syntax::Expr> parsePrimary(lexer::Tokenizer &lexer) noexcept;

/*!\brief Parse RHS expression of binary operator.
 * \param lexer
 * \param lhs The lhs expr to merge rhs with.
 * \param prec Minimal operator precedence.
 */
std::unique_ptr<syntax::Expr> parseRHS(lexer::Tokenizer &lexer,
                                       std::unique_ptr<syntax::Expr> lhs,
                                       std::size_t prec,
                                       bool parseFunctionDecl = false) noexcept;

/*!\brief Parse primary expresion + optional operator where prec is passed
 * to parseRHS.
 * \param lexer
 * \param prec Minimal operator precedence.
 * \see parseRHS
 */
std::unique_ptr<syntax::Expr> parse(lexer::Tokenizer &lexer,
                                    std::size_t prec = 0,
                                    bool parseFunctionDecl = false) noexcept;

/*!\brief Parse program lines.
 * \param lexer
 * \param topLevel True if top level (first scope). False if not.
 * \return Returns parsed program
 */
std::unique_ptr<syntax::Program> parseProgram(lexer::Tokenizer &lexer,
                                              bool topLevel = true) noexcept;

/*!\return Returns true, if currentToken matches tokType (and if
 * curtok == tok_op curop). Otherwise false.
 * \brief If match then advance to next token. If no match, an error messages
 * is printed.
 * \param lex
 * \param tok Will be set to currentToken (if not nullptr).
 * \param tokType
 * \param opType
 */
bool match(lexer::Tokenizer &lex, lexer::Token *tok, lexer::TokenType tokType,
           lexer::OperatorType opType = lexer::op_asg) noexcept;

/*!\return Return true, if currenToken matches one of tokTypes. Otherwise
 * false is returned and also if tokTypes.size() == 0.
 * \brief If match then advance to next token. If no match, an error message is
 * printed.
 * \param lex
 * \param lex Will be set to current Token (if not nullptr).
 * \param tokTypes size() > 0.
 */
bool match(lexer::Tokenizer &lex, lexer::Token *tok,
           const std::vector<lexer::TokenType> &tokTypes) noexcept;

/*!\return Returns an empty vector on failure, otherwise size() >= 1, with
 * strings, which have at least one character.
 * \param lex
 */
std::vector<std::string> parseIdentifierCall(lexer::Tokenizer &lex) noexcept;

/*!\return Returns nullptr on failure, otherwise IdExpr or BiOpExpr.
 * \param lex
 */
std::unique_ptr<syntax::Expr>
parseIdentifierCallExpr(lexer::Tokenizer &lex) noexcept;


/*!\} */
} // end namespace parser
} // end namespace feder

#endif /* FEDER_PARSER_HPP */
