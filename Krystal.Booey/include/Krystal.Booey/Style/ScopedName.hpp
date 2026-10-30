#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/ScopeOrdinal.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::css
{
  class PrimitiveValue;
}

namespace krys::boo::style
{
  /// @brief A name defined or referenced in a tree scope, e.g. a `@keyframes` or container name.
  struct ScopedName
  {
    css::CSSOMStringAtom Name;
    style::ScopeOrdinal ScopeOrdinal {style::ScopeOrdinal::Element};
    bool IsIdentifier {true};

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
      if (IsIdentifier)
      {
        return visitor(css::CustomIdentifier {Name});
      }
      return visitor(Name);
    }

    KRYS_NODISCARD bool operator==(const ScopedName &) const noexcept = default;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ScopedName>
  {
    KRYS_NODISCARD ScopedName operator()(BuilderState &, const css::PrimitiveValue &) noexcept;
    KRYS_NODISCARD ScopedName operator()(BuilderState &, const css::Value &) noexcept;
  };
}

namespace krys
{
  template <>
  struct MarkableTraits<boo::style::ScopedName>
  {
    KRYS_NODISCARD static bool IsEmptyValue(const boo::style::ScopedName &value) noexcept
    {
      return value.Name == boo::css::CSSOMStringAtom::Null();
    }

    KRYS_NODISCARD static boo::style::ScopedName EmptyValue() noexcept
    {
      return boo::style::ScopedName {boo::css::CSSOMStringAtom::Null()};
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ScopedName)
