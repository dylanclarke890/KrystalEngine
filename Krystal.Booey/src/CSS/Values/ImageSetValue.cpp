/*
 * Copyright (C) 2012-2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Values/ImageSetValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageSetOptionValue.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImageSet.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include <numeric>

namespace krys::boo::css
{
  Ref<ImageSetValue> ImageSetValue::Create(ValueListBuilder builder) noexcept
  {
    return AdoptRef(*new ImageSetValue(krys::move(builder)));
  }

  ImageSetValue::ImageSetValue(ValueListBuilder builder) noexcept
      : ValueContainingList(ValueType::ImageSet, ValueSeparator::CommaSeparator, krys::move(builder))
  {
  }

  CSSOMString ImageSetValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result {u8"image-set("};
    for (uint32 i = 0u; i < Length(); ++i)
    {
      if (i > 0u)
      {
        result.append(u8", ");
      }
      krys_debug_assert(Is<ImageSetOptionValue>(Item(i)));
      result.append(Item(i)->CssText(context));
    }
    result.push_back(u8')');
    return result;
  }

  RefPtr<StyleImage> ImageSetValue::CreateStyleImage(const style::BuilderState &state) const noexcept
  {
    auto length = this->Length();

    List<ImageWithScale> images;
    images.reserve(length);
    for (uint32 i = 0; i < length; ++i)
    {
      const auto &option = Downcast<ImageSetOptionValue>(*Item(i));
      images.push_back(ImageWithScale {
        state.CreateStyleImage(option.Image()),
        option.Resolution().ResolveAsResolution<float>(state.CssToLengthConversionData()), option.Type()});
    }

    // Sort the images so that they are stored in order from lowest resolution to highest.
    // We want to maintain the authored order for serialization so we create a sorted indexing vector.
    List<size_t> sortedIndices(length);
    std::iota(sortedIndices.begin(), sortedIndices.end(), 0);

    std::stable_sort(sortedIndices.begin(), sortedIndices.end(), [&images](size_t lhs, size_t rhs)
                     { return images[lhs].ScaleFactor < images[rhs].ScaleFactor; });

    return StyleImageSet::Create(krys::move(images), krys::move(sortedIndices));
  }
}
