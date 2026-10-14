#include "Krystal.Booey/Style/Values/Align/JustifyContent.hpp"

#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  StyleContentAlignmentData
    JustifyContent::Resolve(Maybe<StyleContentAlignmentData> valueForNormal) const noexcept
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
      case PrimaryKind::Normal:       return valueForNormal.value_or(ContentPosition::Normal);
      case PrimaryKind::SpaceBetween: return {ContentPosition::Normal, ContentDistribution::SpaceBetween};
      case PrimaryKind::SpaceAround:  return {ContentPosition::Normal, ContentDistribution::SpaceAround};
      case PrimaryKind::SpaceEvenly:  return {ContentPosition::Normal, ContentDistribution::SpaceEvenly};
      case PrimaryKind::Stretch:      return {ContentPosition::Normal, ContentDistribution::Stretch};
      case PrimaryKind::Center:       return resolveOverflowPosition(ContentPosition::Center);
      case PrimaryKind::Start:        return resolveOverflowPosition(ContentPosition::Start);
      case PrimaryKind::End:          return resolveOverflowPosition(ContentPosition::End);
      case PrimaryKind::FlexStart:    return resolveOverflowPosition(ContentPosition::FlexStart);
      case PrimaryKind::FlexEnd:      return resolveOverflowPosition(ContentPosition::FlexEnd);
      case PrimaryKind::Left:         return resolveOverflowPosition(ContentPosition::Left);
      case PrimaryKind::Right:        return resolveOverflowPosition(ContentPosition::Right);
    }
    krys_unreachable();
  }

  // MARK: - Conversion

  JustifyContent CSSValueConversion<JustifyContent>::operator()(BuilderState &state,
                                                                const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        // <normal>
        case css::ValueId::Normal:       return css::keywords::Normal {};
        // <content-distribution>
        case css::ValueId::SpaceBetween: return css::keywords::SpaceBetween {};
        case css::ValueId::SpaceAround:  return css::keywords::SpaceAround {};
        case css::ValueId::SpaceEvenly:  return css::keywords::SpaceEvenly {};
        case css::ValueId::Stretch:      return css::keywords::Stretch {};
        // <overflow-position>? [ <content-position> | left | right ]
        case css::ValueId::Center:       return css::keywords::Center {};
        case css::ValueId::Start:        return css::keywords::Start {};
        case css::ValueId::End:          return css::keywords::End {};
        case css::ValueId::FlexStart:    return css::keywords::FlexStart {};
        case css::ValueId::FlexEnd:      return css::keywords::FlexEnd {};
        case css::ValueId::Left:         return css::keywords::Left {};
        case css::ValueId::Right:        return css::keywords::Right {};
        default:                         state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    }

    auto pair = RequiredPairDowncast<css::PrimitiveValue>(state, value);
    if (!pair)
    {
      return css::keywords::Normal {};
    }

    auto consumeAfterOverflowPosition = [&](auto overflowPosition, auto secondValueID) -> JustifyContent
    {
      switch (secondValueID)
      {
        case css::ValueId::Center:    return {css::keywords::Center {}, overflowPosition};
        case css::ValueId::Start:     return {css::keywords::Start {}, overflowPosition};
        case css::ValueId::End:       return {css::keywords::End {}, overflowPosition};
        case css::ValueId::FlexStart: return {css::keywords::FlexStart {}, overflowPosition};
        case css::ValueId::FlexEnd:   return {css::keywords::FlexEnd {}, overflowPosition};
        case css::ValueId::Left:      return {css::keywords::Left {}, overflowPosition};
        case css::ValueId::Right:     return {css::keywords::Right {}, overflowPosition};
        default:                      state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
      }
    };

    switch (pair->first->ValueId())
    {
      case css::ValueId::Unsafe:
        return consumeAfterOverflowPosition(css::keywords::Unsafe {}, pair->second->ValueId());
      case css::ValueId::Safe:
        return consumeAfterOverflowPosition(css::keywords::Safe {}, pair->second->ValueId());
      default: state.SetCurrentPropertyInvalidAtComputedValueTime(); return css::keywords::Normal {};
    }
  }
}
