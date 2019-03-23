#ifndef FEDER_PARSER_HPP
#define FEDER_PARSER_HPP

/*!\file feder/parser.hpp
 * \brief Parser
 */

#include "feder/global.hpp"
#include "feder/lexer.hpp"
#include "feder/syntax.hpp"

namespace feder {
  namespace parser {
    /*!\brief Parse primary token.
     */
    std::unique_ptr<syntax::Expr> parsePrimary(lexer::Lexer &lexer) noexcept;

    /*!\brief Parse RHS expression of binary operator.
     * \param lexer
     * \param lhs The lhs expr to merge rhs with.
     * \param prec Minimal operator precedence.
     */
    std::unique_ptr<syntax::Expr> parseRHS(lexer::Lexer &lexer,
        std::unique_ptr<syntax::Expr> lhs, std::size_t prec) noexcept;

    /*!\brief Parse primary expresion + optional operator where prec is passed
     * to parseRHS.
     * \param lexer
     * \param prec Minimal operator precedence.
     * \see parseRHS
     */
    std::unique_ptr<syntax::Expr> parse(lexer::Lexer &lexer,
        std::size_t prec = 0) noexcept;

    /*!\brief Parse program lines.
     * \param lexer
     * \param topLevel True if top level (first scope). False if not.
     * \return Returns parsed program
     */
    std::unique_ptr<syntax::Program> parseProgram(lexer::Lexer &lexer,
        bool topLevel = true) noexcept;
  } // end namespace parser
} // end namespace feder

#endif /* FEDER_PARSER_HPP */
