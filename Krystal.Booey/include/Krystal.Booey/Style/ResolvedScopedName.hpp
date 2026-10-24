#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/ScopeIdentifier.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include <functional>

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::style
{
  struct ScopedName;

  /// @brief A ScopedName that has been resolved to determine which specific scope it belongs to.
  class ResolvedScopedName
  {
  private:
    css::CSSOMStringAtom _name;
    ScopeIdentifier _scopeIdentifier;

  public:
    ResolvedScopedName(css::CSSOMStringAtom name, ScopeIdentifier scopeIdentifier) noexcept
        : _name(krys::move(name)), _scopeIdentifier(scopeIdentifier)
    {
    }

    /// @brief Creates a ResolvedScopedName from a ScopedName by resolving the scope ordinal into the concrete
    /// style scope.
    KRYS_NODISCARD static ResolvedScopedName CreateFromScopedName(const dom::Element &element,
                                                                  const ScopedName &scopedName) noexcept;

    KRYS_NODISCARD const css::CSSOMStringAtom &Name() const noexcept
    {
      return _name;
    }
    KRYS_NODISCARD ScopeIdentifier GetScopeIdentifier() const noexcept
    {
      return _scopeIdentifier;
    }

    KRYS_NODISCARD bool operator==(const ResolvedScopedName &) const noexcept = default;
  };
}

namespace std
{
  template <>
  struct hash<krys::boo::style::ResolvedScopedName>
  {
    size_t operator()(const krys::boo::style::ResolvedScopedName &name) const noexcept
    {
      return krys::UnsignedIntegerPairHash(
        hash<krys::boo::css::CSSOMStringAtom> {}(name.Name()),
        hash<krys::boo::style::ScopeIdentifier> {}(name.GetScopeIdentifier()));
    }
  };
}
