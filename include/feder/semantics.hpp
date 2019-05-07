#ifndef FEDER_SEMANTICS_HPP
#define FEDER_SEMANTICS_HPP

#include "feder/global.hpp"
#include "feder/lexer.hpp"

namespace feder {
namespace semantic {
  //!\brief Is constant semantic
  constexpr std::uint16_t TYPE_CONST =  0x0001;
  //!\brief Is private semantic
  constexpr std::uint16_t TYPE_PRIV  =  0x0002;
  //!\brief Is value. If not it's a type.
  constexpr std::uint16_t TYPE_VAL   =  0x0004;
  //!\brief Is function
  constexpr std::uint16_t TYPE_FUNC  =  0x0010;
  //!\brief Is class
  constexpr std::uint16_t TYPE_CLASS =  0x0020;
  //!\brief Is enum or enum constructor
  constexpr std::uint16_t TYPE_ENUM  =  0x0040;
  //!\brief Is variable.
  constexpr std::uint16_t TYPE_VAR   =  0x0080;
  //!\brief Is array.
  constexpr std::uint16_t TYPE_ARRAY =  0x0100;
  //!\brief Is unique.
  constexpr std::uint16_t TYPE_UNIQUE = 0x0200;
  //!\brief Requires template call
  constexpr std::uint16_t TYPE_TEMPL =  0x0400;
  //!\brief Is this
  constexpr std::uint16_t TYPE_THIS =   0x0800;
  //!\brief Is LHS value
  constexpr std::uint16_t TYPE_LVAL =   0x1000;
  
  //!\brief Type has some issues.
  constexpr std::uint16_t TYPE_ERR   =  0x0000;

  class Type {
    std::uint16_t modifiers;
  public:
    Type(std::uint16_t modifiers) noexcept : modifiers{modifiers} {}
    virtual ~Type() {}

    /*!\return Returns modifiers.
     */
    std::uint16_t getModifiers() const noexcept { return modifiers; }

    /*!\return Return true, if type has all modifiers, otherwise false.
     */
    bool hasModifiers(std::uint16_t modifiers) const noexcept
    { return this->modifiers & modifiers == modifiers; }
  };
} // end namespace semantic
} // end namespace feder

#endif /* FEDER_SEMANTICS_HPP */
