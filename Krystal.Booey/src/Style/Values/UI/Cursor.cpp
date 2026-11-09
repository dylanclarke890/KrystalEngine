/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/UI/Cursor.hpp"

#include "Krystal.Booey/CSS/Values/CursorImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCursorImage.hpp"
#include "Krystal.Booey/Rendering/Style/StyleInvalidImage.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

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

  void Serialiser<CursorImage>::operator()(StringBuilder &builder, const css::SerialisationContext &context,
                                           const RenderStyle &style, const CursorImage &value) noexcept
  {
    builder.Append(value.Image->ComputedStyleValue(style)->CssText(context));
  }
}
