#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::style
{
  /// @brief Identifies a pseudo-element that has its own style.
  struct PseudoElementIdentifier
  {
    PseudoElementType Type;

    /// @brief The highlight name, for `::highlight`.
    css::CSSOMStringAtom NameArgument {css::CSSOMStringAtom::Null()};

    KRYS_NODISCARD friend bool operator==(const PseudoElementIdentifier &,
                                          const PseudoElementIdentifier &) noexcept = default;
  };
}

namespace krys
{
  template <>
  struct DefaultHash<boo::style::PseudoElementIdentifier>
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = true;

    KRYS_NODISCARD static size_t Hash(const boo::style::PseudoElementIdentifier &identifier) noexcept
    {
      return krys::Hash::Combine(identifier.Type, identifier.NameArgument.Storage());
    }

    KRYS_NODISCARD static bool Equal(const boo::style::PseudoElementIdentifier &a,
                                     const boo::style::PseudoElementIdentifier &b) noexcept
    {
      return a == b;
    }
  };
}
