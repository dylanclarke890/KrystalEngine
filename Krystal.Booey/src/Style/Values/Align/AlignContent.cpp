#include "Krystal.Booey/Style/Values/Align/AlignContent.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  StyleContentAlignmentData
    AlignContent::Resolve(Maybe<StyleContentAlignmentData> valueForNormal) const noexcept
  {
    auto resolveOverflowPosition = [&](auto contentPosition) -> StyleContentAlignmentData
    {
      switch (GetOverflowPosition())
      {
        case OverflowPositionKind::None: return {contentPosition, ContentDistribution::Default};
        case OverflowPositionKind::Unsafe:
          return {contentPosition, ContentDistribution::Default, OverflowAlignment::Unsafe};
        case OverflowPositionKind::Safe:
          return {contentPosition, ContentDistribution::Default, OverflowAlignment::Safe};
      }
      krys_unreachable();
    };

    switch (Primary())
    {
      case PrimaryKind::Normal: return valueForNormal.value_or(ContentPosition::Normal);
      case PrimaryKind::Baseline:
        if (GetBaselineAlignmentPreference() == BaselineAlignmentPreferenceKind::Last)
        {
          return {ContentPosition::LastBaseline, ContentDistribution::Default};
        }
        return {ContentPosition::Baseline, ContentDistribution::Default};
      case PrimaryKind::SpaceBetween: return {ContentPosition::Normal, ContentDistribution::SpaceBetween};
      case PrimaryKind::SpaceAround:  return {ContentPosition::Normal, ContentDistribution::SpaceAround};
      case PrimaryKind::SpaceEvenly:  return {ContentPosition::Normal, ContentDistribution::SpaceEvenly};
      case PrimaryKind::Stretch:      return {ContentPosition::Normal, ContentDistribution::Stretch};
      case PrimaryKind::Center:       return resolveOverflowPosition(ContentPosition::Center);
      case PrimaryKind::Start:        return resolveOverflowPosition(ContentPosition::Start);
      case PrimaryKind::End:          return resolveOverflowPosition(ContentPosition::End);
      case PrimaryKind::FlexStart:    return resolveOverflowPosition(ContentPosition::FlexStart);
      case PrimaryKind::FlexEnd:      return resolveOverflowPosition(ContentPosition::FlexEnd);
    }
    krys_unreachable();
  }

  // MARK: - Conversion

  AlignContent CSSValueConversion<AlignContent>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        // <normal>
        case css::ValueId::Normal:       return css::keywords::Normal {};
        // <baseline-position>
        case css::ValueId::Baseline:     return css::keywords::Baseline {};
        // <content-distribution>
        case css::ValueId::SpaceBetween: return css::keywords::SpaceBetween {};
        case css::ValueId::SpaceAround:  return css::keywords::SpaceAround {};
        case css::ValueId::SpaceEvenly:  return css::keywords::SpaceEvenly {};
        case css::ValueId::Stretch:      return css::keywords::Stretch {};
        // <overflow-position>? <content-position>
        case css::ValueId::Start:        return css::keywords::Start {};
        case css::ValueId::End:          return css::keywords::End {};
        case css::ValueId::Center:       return css::keywords::Center {};
        case css::ValueId::FlexStart:    return css::keywords::FlexStart {};
        case css::ValueId::FlexEnd:      return css::keywords::FlexEnd {};
        default:                         state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    auto pair = RequiredPairDowncast<css::PrimitiveValue>(state, value);
    if (!pair)
    {
      return css::keywords::Normal {};
    }

    auto consumeAfterBaselinePositionPreference = [&](auto baselinePositionPreference,
                                                      auto secondValueID) -> AlignContent
    {
      switch (secondValueID)
      {
        case css::ValueId::Baseline: return {css::keywords::Baseline {}, {baselinePositionPreference}};
        default:                     state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    };

    auto consumeAfterOverflowPosition = [&](auto overflowPosition, auto secondValueID) -> AlignContent
    {
      switch (secondValueID)
      {
        case css::ValueId::Start:     return {css::keywords::Start {}, overflowPosition};
        case css::ValueId::End:       return {css::keywords::End {}, overflowPosition};
        case css::ValueId::Center:    return {css::keywords::Center {}, overflowPosition};
        case css::ValueId::FlexStart: return {css::keywords::FlexStart {}, overflowPosition};
        case css::ValueId::FlexEnd:   return {css::keywords::FlexEnd {}, overflowPosition};
        default:                      state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    };

    switch (pair->first->ValueId())
    {
      // <baseline-position>
      case css::ValueId::First:
        return consumeAfterBaselinePositionPreference(css::keywords::First {}, pair->second->ValueId());
      case css::ValueId::Last:
        return consumeAfterBaselinePositionPreference(css::keywords::Last {}, pair->second->ValueId());
      // <overflow-position>? <content-position>
      case css::ValueId::Unsafe:
        return consumeAfterOverflowPosition(css::keywords::Unsafe {}, pair->second->ValueId());
      case css::ValueId::Safe:
        return consumeAfterOverflowPosition(css::keywords::Safe {}, pair->second->ValueId());
      default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
    }
  }
}
