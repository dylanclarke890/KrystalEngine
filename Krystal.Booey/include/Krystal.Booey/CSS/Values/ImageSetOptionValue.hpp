/*
 * Copyright (C) 2023-2025 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief One option of an `image-set()`: an image, its resolution and an optional `type()`.
  class ImageSetOptionValue final : public Value
  {
  private:
    const Ref<Value> _image;
    Ref<PrimitiveValue> _resolution;
    Maybe<CSSOMString> _mimeType;

    ImageSetOptionValue(Ref<Value> &&image, Ref<PrimitiveValue> &&resolution) noexcept;
    ImageSetOptionValue(Ref<Value> &&image, Ref<PrimitiveValue> &&resolution, CSSOMString &&type) noexcept;

  public:
    KRYS_NODISCARD static Ref<ImageSetOptionValue> Create(Ref<Value> &&image) noexcept;
    KRYS_NODISCARD static Ref<ImageSetOptionValue> Create(Ref<Value> &&image,
                                                          Ref<PrimitiveValue> &&resolution) noexcept;
    KRYS_NODISCARD static Ref<ImageSetOptionValue>
      Create(Ref<Value> &&image, Ref<PrimitiveValue> &&resolution, CSSOMString type) noexcept;

    KRYS_NODISCARD bool Equals(const ImageSetOptionValue &other) const noexcept;
    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD Value &Image() const noexcept
    {
      return *_image;
    }

    KRYS_NODISCARD PrimitiveValue &Resolution() const noexcept
    {
      return *_resolution;
    }

    KRYS_NODISCARD Ref<PrimitiveValue> ProtectedResolution() const noexcept
    {
      return _resolution;
    }

    void SetResolution(Ref<PrimitiveValue> &&resolution) noexcept;

    /// @brief The `type()` MIME type, if one was given.
    KRYS_NODISCARD const Maybe<CSSOMString> &Type() const noexcept
    {
      return _mimeType;
    }

    void SetType(CSSOMString type) noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
    {
      if (func(*_image) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      if (func(*_resolution) == IterationStatus::Done)
      {
        return IterationStatus::Done;
      }
      return IterationStatus::Continue;
    }

    KRYS_NODISCARD bool CustomTraverseSubresources(
      KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::ImageSetOptionValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsImageSetOptionValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
