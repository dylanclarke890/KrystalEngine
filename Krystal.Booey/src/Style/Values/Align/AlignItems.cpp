#include "Krystal.Booey/Style/Values/Align/AlignItems.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  StyleSelfAlignmentData AlignItems::Resolve(Maybe<StyleSelfAlignmentData> valueForNormal) const noexcept
  {
    auto resolveOverflowPosition = [&](auto itemPosition) -> StyleSelfAlignmentData
    {
      switch (GetOverflowPosition())
      {
        case OverflowPositionKind::None:   return {itemPosition};
        case OverflowPositionKind::Unsafe: return {itemPosition, OverflowAlignment::Unsafe};
        case OverflowPositionKind::Safe:
          return {
            itemPosition,
            OverflowAlignment::Safe,
          };
      }
      krys_unreachable();
    };

    switch (Primary())
    {
      case PrimaryKind::Normal:  return valueForNormal.value_or(ItemPosition::Normal);
      case PrimaryKind::Stretch: return {ItemPosition::Stretch};
      case PrimaryKind::Baseline:
        if (GetBaselineAlignmentPreference() == BaselineAlignmentPreferenceKind::Last)
        {
          return {ItemPosition::LastBaseline};
        }
        return {ItemPosition::Baseline};
      case PrimaryKind::Center:       return resolveOverflowPosition(ItemPosition::Center);
      case PrimaryKind::Start:        return resolveOverflowPosition(ItemPosition::Start);
      case PrimaryKind::End:          return resolveOverflowPosition(ItemPosition::End);
      case PrimaryKind::SelfStart:    return resolveOverflowPosition(ItemPosition::SelfStart);
      case PrimaryKind::SelfEnd:      return resolveOverflowPosition(ItemPosition::SelfEnd);
      case PrimaryKind::FlexStart:    return resolveOverflowPosition(ItemPosition::FlexStart);
      case PrimaryKind::FlexEnd:      return resolveOverflowPosition(ItemPosition::FlexEnd);
      case PrimaryKind::AnchorCenter: return resolveOverflowPosition(ItemPosition::AnchorCenter);
    }
    krys_unreachable();
  }

  // MARK: - Conversion

  AlignItems CSSValueConversion<AlignItems>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        // normal
        case css::ValueId::Normal:       return css::keywords::Normal {};
        // stretch
        case css::ValueId::Stretch:      return css::keywords::Stretch {};
        // <baseline-position>
        case css::ValueId::Baseline:     return css::keywords::Baseline {};
        // <overflow-position>? <self-position>
        case css::ValueId::Center:       return css::keywords::Center {};
        case css::ValueId::Start:        return css::keywords::Start {};
        case css::ValueId::End:          return css::keywords::End {};
        case css::ValueId::SelfStart:    return css::keywords::SelfStart {};
        case css::ValueId::SelfEnd:      return css::keywords::SelfEnd {};
        case css::ValueId::FlexStart:    return css::keywords::FlexStart {};
        case css::ValueId::FlexEnd:      return css::keywords::FlexEnd {};
        case css::ValueId::AnchorCenter: return css::keywords::AnchorCenter {};
        default:                         state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    auto pair = RequiredPairDowncast<css::PrimitiveValue>(state, value);
    if (!pair)
    {
      return css::keywords::Normal {};
    }

    auto consumeAfterBaselinePositionPreference = [&](auto baselinePositionPreference,
                                                      auto secondValueID) -> AlignItems
    {
      switch (secondValueID)
      {
        case css::ValueId::Baseline: return {css::keywords::Baseline {}, {baselinePositionPreference}};
        default:                     state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    };

    auto consumeAfterOverflowPosition = [&](auto overflowPosition, auto secondValueID) -> AlignItems
    {
      switch (secondValueID)
      {
        case css::ValueId::Start:        return {css::keywords::Start {}, overflowPosition};
        case css::ValueId::End:          return {css::keywords::End {}, overflowPosition};
        case css::ValueId::Center:       return {css::keywords::Center {}, overflowPosition};
        case css::ValueId::SelfStart:    return {css::keywords::SelfStart {}, overflowPosition};
        case css::ValueId::SelfEnd:      return {css::keywords::SelfEnd {}, overflowPosition};
        case css::ValueId::FlexStart:    return {css::keywords::FlexStart {}, overflowPosition};
        case css::ValueId::FlexEnd:      return {css::keywords::FlexEnd {}, overflowPosition};
        case css::ValueId::AnchorCenter: return {css::keywords::AnchorCenter {}, overflowPosition};
        default:                         state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    };

    switch (pair->first->ValueId())
    {
      // <baseline-position>
      case css::ValueId::First:
        return consumeAfterBaselinePositionPreference(css::keywords::First {}, pair->second->ValueId());
      case css::ValueId::Last:
        return consumeAfterBaselinePositionPreference(css::keywords::Last {}, pair->second->ValueId());
      // <overflow-position>? <self-position>
      case css::ValueId::Unsafe:
        return consumeAfterOverflowPosition(css::keywords::Unsafe {}, pair->second->ValueId());
      case css::ValueId::Safe:
        return consumeAfterOverflowPosition(css::keywords::Safe {}, pair->second->ValueId());
      default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
    }
  }
}
