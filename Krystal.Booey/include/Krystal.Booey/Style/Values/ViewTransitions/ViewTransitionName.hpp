#pragma once

#include "Krystal.Booey/Style/ScopeOrdinal.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'view-transition-name> = none | auto | match-element | <custom-ident excluding=none,auto,match-element>
  // https://www.w3.org/TR/css-view-transitions-1/#propdef-view-transition-name
  struct ViewTransitionName
  {
    ViewTransitionName(css::keywords::None keyword) noexcept : _value {keyword}
    {
    }

    ViewTransitionName(css::keywords::Auto keyword, ScopeOrdinal ordinal) noexcept
        : _value {keyword}, _scopeOrdinal {ordinal}
    {
    }

    ViewTransitionName(css::keywords::MatchElement keyword, ScopeOrdinal ordinal) noexcept
        : _value {keyword}, _scopeOrdinal {ordinal}
    {
    }

    ViewTransitionName(css::CustomIdentifier &&customIdentifier, ScopeOrdinal ordinal) noexcept
        : _value {krys::move(customIdentifier)}, _scopeOrdinal {ordinal}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::None>(_value);
    }
    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Auto>(_value);
    }
    KRYS_NODISCARD bool IsMatchElement() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::MatchElement>(_value);
    }
    KRYS_NODISCARD bool IsCustomIdentifier() const noexcept
    {
      return krys::HoldsAlternative<css::CustomIdentifier>(_value);
    }

    KRYS_NODISCARD style::ScopeOrdinal GetScopeOrdinal() const noexcept
    {
      krys_debug_assert(!IsNone());
      return _scopeOrdinal;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const ViewTransitionName &) const noexcept = default;

  private:
    Variant<css::keywords::None, css::keywords::Auto, css::keywords::MatchElement, css::CustomIdentifier>
      _value;
    style::ScopeOrdinal _scopeOrdinal {style::ScopeOrdinal::Element};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ViewTransitionName>
  {
    KRYS_NODISCARD ViewTransitionName operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ViewTransitionName)
