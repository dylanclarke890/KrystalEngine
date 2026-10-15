#include "Krystal.Booey/Style/Values/Animations/SingleAnimationRange.hpp"

#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperDeprecatedCSSValueConversion.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  SingleAnimationRangeLength SingleAnimationRangeLength::DefaultValue(SingleAnimationRangeType type) noexcept
  {
    return type == SingleAnimationRangeType::Start ? 0_css_percentage : 100_css_percentage;
  }

  bool SingleAnimationRangeLength::IsDefault(SingleAnimationRangeType type) const noexcept
  {
    return *this == DefaultValue(type);
  }

  SingleAnimationRange SingleAnimationRange::DefaultForScrollTimeline() noexcept
  {
    return {
      .Start = {0_css_percentage},
      .End = {100_css_percentage},
    };
  }

  SingleAnimationRange SingleAnimationRange::DefaultForViewTimeline() noexcept
  {
    return {
      .Start = {css::keywords::Cover {}, 0_css_percentage},
      .End = {css::keywords::Cover {}, 100_css_percentage},
    };
  }

  // MARK: - Conversion

  template <typename Edge>
  KRYS_NODISCARD static Edge ConvertSingleAnimationRangeEdge(BuilderState &state,
                                                             const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Normal:        return css::keywords::Normal {};
        case css::ValueId::Cover:         return css::keywords::Cover {};
        case css::ValueId::Contain:       return css::keywords::Contain {};
        case css::ValueId::Entry:         return css::keywords::Entry {};
        case css::ValueId::Exit:          return css::keywords::Exit {};
        case css::ValueId::EntryCrossing: return css::keywords::EntryCrossing {};
        case css::ValueId::ExitCrossing:  return css::keywords::ExitCrossing {};
        default:                          break;
      }

      return ConvertCSSValueToStyle<SingleAnimationRangeLength>(state, *primitiveValue);
    }

    auto pair = RequiredPairDowncast<css::PrimitiveValue>(state, value);
    if (!pair)
    {
      return css::keywords::Normal {};
    }

    auto offset = ConvertCSSValueToStyle<SingleAnimationRangeLength>(state, *pair->second);

    switch (pair->first->ValueId())
    {
      case css::ValueId::Cover:         return {css::keywords::Cover {}, krys::move(offset)};
      case css::ValueId::Contain:       return {css::keywords::Contain {}, krys::move(offset)};
      case css::ValueId::Entry:         return {css::keywords::Entry {}, krys::move(offset)};
      case css::ValueId::Exit:          return {css::keywords::Exit {}, krys::move(offset)};
      case css::ValueId::EntryCrossing: return {css::keywords::EntryCrossing {}, krys::move(offset)};
      case css::ValueId::ExitCrossing:  return {css::keywords::ExitCrossing {}, krys::move(offset)};
      default:                          break;
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::Normal {};
  }

  SingleAnimationRangeStart
    CSSValueConversion<SingleAnimationRangeStart>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    return ConvertSingleAnimationRangeEdge<SingleAnimationRangeStart>(state, value);
  }

  SingleAnimationRangeEnd
    CSSValueConversion<SingleAnimationRangeEnd>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    return ConvertSingleAnimationRangeEdge<SingleAnimationRangeEnd>(state, value);
  }

  // MARK: - Deprecated Conversions

  template <typename Edge>
  KRYS_NODISCARD static Maybe<Edge>
    DeprecatedConvertSingleAnimationRangeEdge(const RefPtr<dom::Element> &element,
                                              const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Cover:         return {css::keywords::Cover {}};
        case css::ValueId::Contain:       return {css::keywords::Contain {}};
        case css::ValueId::Entry:         return {css::keywords::Entry {}};
        case css::ValueId::Exit:          return {css::keywords::Exit {}};
        case css::ValueId::EntryCrossing: return {css::keywords::EntryCrossing {}};
        case css::ValueId::ExitCrossing:  return {css::keywords::ExitCrossing {}};
        default:                          break;
      }

      auto offset = DeprecatedConvertCSSValueToStyle<SingleAnimationRangeLength>(element, *primitiveValue);
      if (!offset)
      {
        return {};
      }

      return {krys::move(*offset)};
    }

    const auto *pair = DynamicDowncast<css::ValuePair>(value);
    if (!pair)
    {
      return {};
    }

    const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(pair->Second());
    if (!primitiveValue)
    {
      return {};
    }

    auto offset = DeprecatedConvertCSSValueToStyle<SingleAnimationRangeLength>(element, *primitiveValue);
    if (!offset)
    {
      return {};
    }

    switch (pair->First().ValueId())
    {
      case css::ValueId::Cover:         return {{css::keywords::Cover {}, krys::move(offset)}};
      case css::ValueId::Contain:       return {{css::keywords::Contain {}, krys::move(offset)}};
      case css::ValueId::Entry:         return {{css::keywords::Entry {}, krys::move(offset)}};
      case css::ValueId::Exit:          return {{css::keywords::Exit {}, krys::move(offset)}};
      case css::ValueId::EntryCrossing: return {{css::keywords::EntryCrossing {}, krys::move(offset)}};
      case css::ValueId::ExitCrossing:  return {{css::keywords::ExitCrossing {}, krys::move(offset)}};
      default:                          break;
    }

    return {};
  }

  Maybe<SingleAnimationRangeStart>
    DeprecatedCSSValueConversion<SingleAnimationRangeStart>::operator()(const RefPtr<dom::Element> &element,
                                                                        const css::Value &value) noexcept
  {
    return DeprecatedConvertSingleAnimationRangeEdge<SingleAnimationRangeStart>(element, value);
  }

  Maybe<SingleAnimationRangeEnd>
    DeprecatedCSSValueConversion<SingleAnimationRangeEnd>::operator()(const RefPtr<dom::Element> &element,
                                                                      const css::Value &value) noexcept
  {
    return DeprecatedConvertSingleAnimationRangeEdge<SingleAnimationRangeEnd>(element, value);
  }
}
