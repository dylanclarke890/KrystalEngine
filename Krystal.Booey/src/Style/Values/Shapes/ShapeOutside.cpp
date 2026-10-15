#include "Krystal.Booey/Style/Values/Shapes/ShapeOutside.hpp"

#include "Krystal.Booey/CSS/Values/BasicShapeValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  bool ShapeOutside::Image::IsValid() const noexcept
  {
    const auto &styleImage = Value.Value;
    if (styleImage->HasCachedImage())
    {
      // FIXME: Also require the cached image to have loaded an image, once the resource loader is ported.
      return styleImage->GetCachedImage() != nullptr;
    }
    return styleImage->IsGeneratedImage();
  }

  // MARK: - Conversion

  ShapeOutside CSSValueConversion<ShapeOutside>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      if (primitiveValue->ValueId() == css::ValueId::None)
      {
        return css::keywords::None {};
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::None {};
    }

    if (value.IsImage())
    {
      return ShapeOutside::Image {ImageWrapper {Ref<StyleImage> {state.CreateStyleImage(value)}}};
    }

    Maybe<BasicShape> shape;
    auto referenceBox = CSSBoxType::BoxMissing;

    auto processSingleValue = [&](const css::Value &currentValue)
    {
      if (auto *shapeValue = DynamicDowncast<css::BasicShapeValue>(currentValue))
      {
        shape = ConvertToStyle(shapeValue->GetShape(), state, 1.0f);
      }
      else
      {
        referenceBox = ConvertCSSValueToStyle<CSSBoxType>(state, currentValue);
      }
    };

    if (auto *list = DynamicDowncast<css::ValueList>(value))
    {
      for (const auto &currentValue : *list)
      {
        processSingleValue(currentValue);
      }
    }
    else
    {
      processSingleValue(value);
    }

    if (shape)
    {
      if (referenceBox != CSSBoxType::BoxMissing)
      {
        return ShapeOutside::ShapeAndShapeBox {.Shape = krys::move(*shape), .Box = referenceBox};
      }
      return ShapeOutside::Shape {krys::move(*shape)};
    }

    if (referenceBox != CSSBoxType::BoxMissing)
    {
      return referenceBox;
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::None {};
  }

  // MARK: - Blending

  bool Blending<ShapeOutside>::CanBlend(const ShapeOutside &a, const ShapeOutside &b) noexcept
  {
    RefPtr aValue = a._value;
    RefPtr bValue = b._value;

    if (!aValue || !bValue)
    {
      return false;
    }

    return std::visit(krys::CreateVisitor(
                        [](const ShapeOutside::ShapeAndShapeBox &a, const ShapeOutside::ShapeAndShapeBox &b)
                        { return style::CanBlend(a.Shape, b.Shape) && a.Box == b.Box; },
                        [](const ShapeOutside::Shape &a, const ShapeOutside::Shape &b)
                        { return style::CanBlend(a, b); }, [](const auto &, const auto &) { return false; }),
                      aValue->Contents, bValue->Contents);
  }

  ShapeOutside Blending<ShapeOutside>::Blend(const ShapeOutside &a, const ShapeOutside &b,
                                             const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(CanBlend(a, b));

    RefPtr aValue = a._value;
    RefPtr bValue = b._value;

    return std::visit(
      krys::CreateVisitor(
        [&](const ShapeOutside::ShapeAndShapeBox &a, const ShapeOutside::ShapeAndShapeBox &b) -> ShapeOutside
        {
          return ShapeOutside::ShapeAndShapeBox {.Shape = style::Blend(a.Shape, b.Shape, context),
                                                 .Box = a.Box};
        },
        [&](const ShapeOutside::Shape &a, const ShapeOutside::Shape &b) -> ShapeOutside
        { return style::Blend(a, b, context); },
        [](const auto &, const auto &) -> ShapeOutside { krys_unreachable(); }),
      aValue->Contents, bValue->Contents);
  }
}
