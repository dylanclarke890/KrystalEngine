#include "Krystal.Booey/Style/Values/Backgrounds/BackgroundSize.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  BackgroundSize CSSValueConversion<BackgroundSize>::operator()(BuilderState &state,
                                                                const css::Value &value) noexcept
  {
    if (auto *pair = DynamicDowncast<css::ValuePair>(value))
    {
      return BackgroundSize::LengthSize {
        ConvertCSSValueToStyle<BackgroundSizeLength>(state, pair->First()),
        ConvertCSSValueToStyle<BackgroundSizeLength>(state, pair->Second()),
      };
    }

    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::Auto {};
    }

    if (primitiveValue->IsValueId())
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::Auto:    return css::keywords::Auto {};
        case css::ValueId::Cover:   return css::keywords::Cover {};
        case css::ValueId::Contain: return css::keywords::Contain {};
        default:                    break;
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::Auto {};
    }

    return ConvertCSSValueToStyle<BackgroundSizeLength>(state, *primitiveValue);
  }

  Ref<css::Value> CSSValueCreation<BackgroundSize>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                               const BackgroundSize &value) noexcept
  {
    return krys::SwitchOn(
      value, [&](const css::keywords::Cover &keyword) -> Ref<css::Value>
      { return CreateCSSValue(pool, style, keyword); },
      [&](const css::keywords::Contain &keyword) -> Ref<css::Value>
      { return CreateCSSValue(pool, style, keyword); },
      [&](const BackgroundSize::LengthSize &lengthSize) -> Ref<css::Value>
      {
        if (lengthSize.Width().IsAuto() && lengthSize.Height().IsAuto())
        {
          return CreateCSSValue(pool, style, css::keywords::Auto {});
        }
        else
        {
          return CreateCSSValue(pool, style, lengthSize);
        }
      });
  }

  // MARK: - Serialization

  void Serialiser<BackgroundSize>::operator()(StringBuilder &builder,
                                              const css::SerialisationContext &context,
                                              const RenderStyle &style, const BackgroundSize &value) noexcept
  {
    krys::SwitchOn(
      value, [&](const css::keywords::Cover &keyword) { Serialise(builder, context, style, keyword); },
      [&](const css::keywords::Contain &keyword) { Serialise(builder, context, style, keyword); },
      [&](const BackgroundSize::LengthSize &lengthSize)
      {
        // FIXME: This should probably serialize just the first value if the second is `auto` but this
        // currently causes a WPT test to fail.
        if (lengthSize.Width().IsAuto() && lengthSize.Height().IsAuto())
        {
          Serialise(builder, context, style, css::keywords::Auto {});
        }
        else
        {
          Serialise(builder, context, style, lengthSize);
        }
      });
  }

  // MARK: - Blending

  bool Blending<BackgroundSize>::CanBlend(const BackgroundSize &a, const BackgroundSize &b) noexcept
  {
    if (!a.HasSameType(b))
    {
      return false;
    }

    if (!a.IsLengthSize())
    {
      return true;
    }

    return style::CanBlend(a.TryLengthSize()->Width(), b.TryLengthSize()->Width())
           && style::CanBlend(a.TryLengthSize()->Height(), b.TryLengthSize()->Height());
  }

  BackgroundSize Blending<BackgroundSize>::Blend(const BackgroundSize &a, const BackgroundSize &b,
                                                 const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(a.HasSameType(b));

    if (!a.IsLengthSize())
    {
      return context.Progress ? b : a;
    }

    return BackgroundSize::LengthSize {
      style::Blend(a.TryLengthSize()->Width(), b.TryLengthSize()->Width(), context),
      style::Blend(a.TryLengthSize()->Height(), b.TryLengthSize()->Height(), context),
    };
  }
}
