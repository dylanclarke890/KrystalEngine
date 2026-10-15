#include "Krystal.Booey/Style/Values/UI/Cursor.hpp"

#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/CursorImageValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCursorImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleInvalidImage.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  Cursor CSSValueConversion<Cursor>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (Is<css::PrimitiveValue>(value))
    {
      return Cursor {FromCSSValue<CursorType>(value)};
    }

    auto list = RequiredListDowncast<css::ValueList, css::Value, 2>(state, value);
    if (!list)
    {
      return css::keywords::Auto {};
    }

    auto images = CursorImageList::createWithSizeFromGenerator(
      list->size() - 1,
      [&](auto index)
      {
        const auto &item = list->item(static_cast<uint32>(index));
        auto *image = RequiredDowncast<css::CursorImageValue>(state, item);
        if (!image)
        {
          return CursorImage {StyleInvalidImage::Create()};
        }

        auto styleImage = image->CreateStyleImage(state);
        if (!styleImage)
        {
          state.SetCurrentPropertyInvalidAtComputedValueTime();
          return CursorImage {StyleInvalidImage::Create()};
        }

        // Point outside the image is how we tell the cursor machinery there is no hot spot, and it should
        // generate one (done in the Cursor class).
        // FIXME: Would it be better to extend the concept of "no hot spot" deeper, into CursorImage and
        // beyond, rather than using -1/-1 for it?
        auto hotSpot = styleImage->HotSpot().value_or(IntPoint {-1, -1});
        return CursorImage {Ref<StyleImage> {krys::move(styleImage)}, hotSpot};
      });

    return {krys::move(images), FromCSSValue<CursorType>(list->item(list->size() - 1))};
  }

  Ref<css::Value> CSSValueCreation<CursorImage>::operator()(css::ValuePool &, const RenderStyle &style,
                                                            const CursorImage &value) noexcept
  {
    return value.Image->ComputedStyleValue(style);
  }

  // MARK: - Serialization

  void Serialiser<CursorImage>::operator()(css::CSSOMString &builder,
                                           const css::SerialisationContext &context, const RenderStyle &style,
                                           const CursorImage &value) noexcept
  {
    builder.append(value.Image->ComputedStyleValue(style)->CssText(context));
  }
}
