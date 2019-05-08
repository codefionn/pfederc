#ifndef FEDER_SEMANTICS_HPP
#define FEDER_SEMANTICS_HPP

/*!\file feder/semantics.hpp
 */

#include "feder/global.hpp"
#include "feder/lexer.hpp"

namespace feder {
namespace semantic {
  typedef std::uint16_t type_semantic;

  //!\brief Is constant semantic
  constexpr type_semantic TYPE_CONST =  0x0001;
  //!\brief Is private semantic
  constexpr type_semantic TYPE_PRIV  =  0x0002;
  //!\brief Is value. If not it's a type.
  constexpr type_semantic TYPE_VAL   =  0x0004;
  //!\brief Is function
  constexpr type_semantic TYPE_FUNC  =  0x0010;
  //!\brief Is class
  constexpr type_semantic TYPE_CLASS =  0x0020;
  //!\brief Is enum or enum constructor
  constexpr type_semantic TYPE_ENUM  =  0x0040;
  //!\brief Is variable.
  constexpr type_semantic TYPE_VAR   =  0x0080;
  //!\brief Is array.
  constexpr type_semantic TYPE_ARRAY =  0x0100;
  //!\brief Is unique.
  constexpr type_semantic TYPE_UNIQUE = 0x0200;
  //!\brief Requires template call
  constexpr type_semantic TYPE_TEMPL =  0x0400;
  //!\brief Is this
  constexpr type_semantic TYPE_THIS =   0x0800;
  //!\brief Is LHS value
  constexpr type_semantic TYPE_LVAL =   0x1000;
  //!\brief Tuples
  constexpr type_semantic TYPE_TUPLE =  0x2000;
  
  //!\brief Type has some issues.
  constexpr type_semantic TYPE_ERR   =  0x0000;

  class Type {
    Type *parent;
    type_semantic modifiers;
  public:
    Type(type_semantic modifiers,
         Type *parent = nullptr) noexcept
      : modifiers{modifiers}, parent{parent} {}
    virtual ~Type() {}

    /*!\return Returns modifiers.
     */
    type_semantic getModifiers() const noexcept { return modifiers; }

    /*!\return Return true, if type has all modifiers, otherwise false.
     */
    bool hasModifiers(type_semantic modifiers) const noexcept
    { return this->modifiers & modifiers == modifiers; }

    /*!\return Returns optional parent.
     */
    Type *getParent() const noexcept { return parent; }

    /*!\return Returns true if getParent() != nullptr, otherwise false.
     */
    bool hasParent() const noexcept { return (bool) parent; }
  };
} // end namespace semantic
} // end namespace feder

#endif /* FEDER_SEMANTICS_HPP */
