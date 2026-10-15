#include "Krystal.Booey/Style/Values/AnchorPosition/PositionTryFallback.hpp"
#include "Krystal.Booey/CSS/Parser/ParserMode.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"

namespace krys::boo::style
{
  bool PositionTryFallback::PositionArea::operator==(
    const PositionTryFallback::PositionArea &other) const noexcept
  {
    if (Properties && other.Properties)
    {
      if (Properties == other.Properties)
      {
        return true;
      }

      auto lhsPositionArea = Properties->GetPropertyCSSValue(css::PropertyId::PositionArea);
      krys_debug_assert(lhsPositionArea);
      auto rhsPositionArea = other.Properties->GetPropertyCSSValue(css::PropertyId::PositionArea);
      krys_debug_assert(rhsPositionArea);

      return *lhsPositionArea == *rhsPositionArea;
    }

    return !Properties && !other.Properties;
  }

  bool PositionTryFallback::operator==(const PositionTryFallback &other) const noexcept
  {
    if (PositionAreaFallback.Properties && other.PositionAreaFallback.Properties)
    {
      return PositionAreaFallback == other.PositionAreaFallback;
    }

    if (!PositionAreaFallback.Properties && !other.PositionAreaFallback.Properties)
    {
      return RuleAndTacticsFallback == other.RuleAndTacticsFallback;
    }

    // If we got here, this and other don't have the same type (e.g comparing position-area with rule +
    // tactics)
    return false;
  }

  // MARK: - Conversion

  PositionTryFallback CSSValueConversion<PositionTryFallback>::operator()(BuilderState &state,
                                                                          const css::Value &value) noexcept
  {
    if (const auto *valueList = DynamicDowncast<css::ValueList>(value))
    {
      if (valueList->Separator() != css::ValueSeparator::SpaceSeparator)
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return {};
      }

      auto rule = Maybe<ScopedName> {};
      auto tactics = css::SpaceSeparatedList<PositionTryFallbackTactic> {};

      for (const auto &item : *valueList)
      {
        switch (item.ValueId())
        {
          case css::ValueId::FlipBlock: tactics.Value.push_back(PositionTryFallbackTactic::FlipBlock); break;
          case css::ValueId::FlipInline:
            tactics.Value.push_back(PositionTryFallbackTactic::FlipInline);
            break;
          case css::ValueId::FlipStart: tactics.Value.push_back(PositionTryFallbackTactic::FlipStart); break;
          case css::ValueId::FlipX:     tactics.Value.push_back(PositionTryFallbackTactic::FlipX); break;
          case css::ValueId::FlipY:     tactics.Value.push_back(PositionTryFallbackTactic::FlipY); break;
          case css::ValueId::Invalid:
          {
            const auto *primitive = DynamicDowncast<css::PrimitiveValue>(item);
            if (primitive && primitive->IsCustomIdent() && !rule)
            {
              rule = ScopedName {css::CSSOMStringAtom {primitive->StringValue()}, state.StyleScopeOrdinal()};
              break;
            }
            state.SetCurrentPropertyInvalidAtComputedValueTime();
            return {};
          }
          default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return {};
        }
      }

      PositionTryFallback result;
      result.RuleAndTacticsFallback.Rule = krys::move(rule);
      if (!tactics.isEmpty())
      {
        result.RuleAndTacticsFallback.Tactics = krys::move(tactics);
      }
      return result;
    }

    // Turn the inlined position-area fallback into properties object that can be applied similarly to
    // @position-try declarations.
    css::Property property {css::PropertyId::PositionArea, ShareRef(const_cast<css::Value &>(value))};
    PositionTryFallback result;
    result.PositionAreaFallback.Properties = css::ImmutableStyleProperties::CreateDeduplicating(
      Span<const css::Property> {&property, 1}, css::ParserMode::HTMLStandard);
    return result;
  }

  Ref<css::Value> CSSValueCreation<PositionTryFallback::PositionArea>::operator()(
    css::ValuePool &, const RenderStyle &, const PositionTryFallback::PositionArea &value) noexcept
  {
    return Ref<css::Value> {value.Properties->GetPropertyCSSValue(css::PropertyId::PositionArea)};
  }

  // MARK: - Serialization

  void Serialiser<PositionTryFallback::PositionArea>::operator()(
    css::CSSOMString &builder, const css::SerialisationContext &context, const RenderStyle &,
    const PositionTryFallback::PositionArea &value) noexcept
  {
    builder.append(value.Properties->GetPropertyCSSValue(css::PropertyId::PositionArea)->CssText(context));
  }
}
