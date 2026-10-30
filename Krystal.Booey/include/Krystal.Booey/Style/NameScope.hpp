#pragma once

#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/Style/ScopeOrdinal.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Visitor.hpp"

#include <algorithm>

namespace krys::boo::style
{
  // <name-scope> = none | all | <dashed-ident>#
  // Used by:
  // <'timeline-scope'>
  // https://drafts.csswg.org/scroll-animations-1/#propdef-timeline-scope
  // <'anchor-scope'>
  // https://drafts.csswg.org/css-anchor-position-1/#propdef-anchor-scope
  struct NameScope
  {
    enum class Type : uint8
    {
      None,
      All,
      Ident
    };

    NameScope(css::keywords::None) noexcept
    {
    }

    NameScope(css::keywords::All, style::ScopeOrdinal scopeOrdinal) noexcept
        : Type {NameScope::Type::All}, ScopeOrdinal {scopeOrdinal}
    {
    }

    NameScope(css::CommaSeparatedListHashSet<css::CustomIdentifier> &&names,
              style::ScopeOrdinal scopeOrdinal) noexcept
        : Type {NameScope::Type::Ident}, Names {krys::move(names)}, ScopeOrdinal {scopeOrdinal}
    {
    }

    NameScope::Type Type {NameScope::Type::None};
    css::CommaSeparatedListHashSet<css::CustomIdentifier> Names;
    style::ScopeOrdinal ScopeOrdinal {style::ScopeOrdinal::Element};

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (Type)
      {
        case NameScope::Type::None: return visitor(css::keywords::None {});
        case NameScope::Type::All:  return visitor(css::keywords::All {});
        case NameScope::Type::Ident:
          if (Names.isEmpty())
          {
            return visitor(css::keywords::None {});
          }
          return visitor(Names);
      }
      krys_unreachable();
    }

    KRYS_NODISCARD bool operator==(const NameScope &other) const noexcept
    {
      return Type == other.Type
             && ScopeOrdinal == other.ScopeOrdinal
             // Two name lists are equal if they contain the same values in the same order.
             // FIXME: This is not symmetrical in the case that other.Names.isEmpty() is true, but
             // Names.isEmpty() is false.
             && (Names.isEmpty() || std::ranges::equal(Names, other.Names));
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<NameScope>
  {
    KRYS_NODISCARD NameScope operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::NameScope)
