#pragma once

#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::style
{
  /// @brief The pseudo-element whose rules are being collected.
  class PseudoElementRequest
  {
  private:
    PseudoElementIdentifier _identifier;

  public:
    explicit PseudoElementRequest(PseudoElementType type) noexcept : _identifier({type})
    {
    }

    PseudoElementRequest(PseudoElementType type, const css::CSSOMStringAtom &nameArgument) noexcept
        : _identifier({type, nameArgument})
    {
      krys_debug_assert(type == PseudoElementType::Highlight || type == PseudoElementType::ViewTransitionGroup
                        || type == PseudoElementType::ViewTransitionImagePair
                        || type == PseudoElementType::ViewTransitionOld
                        || type == PseudoElementType::ViewTransitionNew);
    }

    explicit PseudoElementRequest(const PseudoElementIdentifier &pseudoElementIdentifier) noexcept
        : _identifier(pseudoElementIdentifier)
    {
    }

    KRYS_NODISCARD const PseudoElementIdentifier &Identifier() const noexcept
    {
      return _identifier;
    }

    KRYS_NODISCARD PseudoElementType GetType() const noexcept
    {
      return _identifier.Type;
    }

    KRYS_NODISCARD const css::CSSOMStringAtom &NameArgument() const noexcept
    {
      return _identifier.NameArgument;
    }
  };
}
