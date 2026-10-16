#pragma once

#include "Krystal.Booey/CSS/Properties/ImmutableStyleProperties.hpp"
#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/Style/Values/AnchorPosition/PositionTryFallbackTactic.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::style
{
  // <position-try-fallback> = [ [<dashed-ident> || <try-tactic>] | <position-area> ]
  // https://drafts.csswg.org/css-anchor-position-1/#propdef-position-try-fallbacks
  struct PositionTryFallback
  {
    using Tactic = PositionTryFallbackTactic;

    struct RuleAndTactics
    {
      Markable<ScopedName> Rule {};
      css::ListOrNull<css::SpaceSeparatedList<Tactic>> Tactics {std::nullopt};

      KRYS_NODISCARD bool operator==(const RuleAndTactics &) const noexcept = default;
    };

    struct PositionArea
    {
      RefPtr<const css::ImmutableStyleProperties> Properties {};

      KRYS_NODISCARD bool operator==(const PositionArea &) const noexcept;
    };

    // Only one of these is valid at a time.
    // FIXME: Use a Variant<PositionTryFallback::RuleAndTactics, PositionTryFallback::PositionArea> to enforce
    // this invariant, and then make PositionArea use a Ref instead of a RefPtr.
    RuleAndTactics RuleAndTacticsFallback {};
    PositionArea PositionAreaFallback {};

    KRYS_NODISCARD bool IsPositionArea() const noexcept
    {
      return !!PositionAreaFallback.Properties;
    }
    KRYS_NODISCARD bool IsRuleAndTactics() const noexcept
    {
      return !PositionAreaFallback.Properties;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsPositionArea())
      {
        return visitor(PositionAreaFallback);
      }
      return visitor(RuleAndTacticsFallback);
    }

    KRYS_NODISCARD bool operator==(const PositionTryFallback &) const noexcept;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const PositionTryFallback::RuleAndTactics &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Rule;
    }
    else if constexpr (I == 1)
    {
      return value.Tactics;
    }
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<PositionTryFallback>
  {
    KRYS_NODISCARD PositionTryFallback operator()(BuilderState &, const css::Value &) noexcept;
  };
  // `PositionTryFallback::PositionArea` is special-cased to handle `ImmutableStyleProperties`.
  template <>
  struct CSSValueCreation<PositionTryFallback::PositionArea>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const PositionTryFallback::PositionArea &) noexcept;
  };

  // MARK: - Serialization

  // `PositionTryFallback::PositionArea` is special-cased to handle `ImmutableStyleProperties`.
  template <>
  struct Serialiser<PositionTryFallback::PositionArea>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const PositionTryFallback::PositionArea &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::PositionTryFallback)
DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::PositionTryFallback::RuleAndTactics, 2)
