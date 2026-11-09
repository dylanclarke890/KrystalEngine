/*
 * Copyright (C) 2011, 2012 Apple Inc. All rights reserved.
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
#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Utils/AlignedStorage.hpp"

namespace krys::boo::css
{
  class ValueList;
  class ValuePool;

  /// @brief The values shared by every pool: keywords, small integers, and the most common colours.
  class StaticValuePool
  {
    friend class PrimitiveValue;
    friend class ValuePool;
    friend class LazyNeverDestroyed<StaticValuePool>;

  private:
    constexpr static size_t MaximumCacheableIntegerValue = 255uz;

    PrimitiveValue _implicitInitialValue;

    ColorValue _transparentColor;
    ColorValue _whiteColor;
    ColorValue _blackColor;

    Array<AlignedStorage<PrimitiveValue>, MaximumCacheableIntegerValue + 1uz> _pixelValues;
    Array<AlignedStorage<PrimitiveValue>, MaximumCacheableIntegerValue + 1uz> _percentageValues;
    Array<AlignedStorage<PrimitiveValue>, MaximumCacheableIntegerValue + 1uz> _numberValues;
    Array<AlignedStorage<PrimitiveValue>, TotalValueKeywords> _identifierValues;

    StaticValuePool() noexcept;

  public:
    static void Init() noexcept;
  };

  extern LazyNeverDestroyed<StaticValuePool> CommonValuePool;

  /// @brief Caches values that are expensive to create or commonly repeated.
  class ValuePool
  {
    KRYS_NON_COPYABLE(ValuePool);

    // Lets `HashMap` use `DefaultHash<boo::Color>`.
    struct ColorHash
    {
      KRYS_NODISCARD size_t operator()(const boo::Color &color) const noexcept
      {
        return DefaultHash<boo::Color>::Hash(color);
      }
    };

  private:
    HashMap<boo::Color, Ref<ColorValue>, ColorHash> _colorValueCache;
    HashMap<CSSOMStringAtom, RefPtr<ValueList>> _fontFaceValueCache;
    HashMap<CSSOMStringAtom, Ref<PrimitiveValue>> _fontFamilyValueCache;

  public:
    ValuePool() noexcept;
    ~ValuePool() noexcept;

    // FIXME: This function needs a name that make it clear that this is not the one and only CSSValuePool,
    // rather the one that can be used only on the main thread.
    // FIXME: Consider a design where the value pool thread-local storage so callers don't have to deal
    // directly with the value pool at all.
    KRYS_NODISCARD static ValuePool &MainThreadPool() noexcept;

    void Drain() noexcept;

    KRYS_NODISCARD Ref<ColorValue> CreateColorValue(const boo::Color &color) noexcept;
    KRYS_NODISCARD RefPtr<ValueList> CreateFontFaceValue(const CSSOMStringAtom &string) noexcept;
    KRYS_NODISCARD Ref<PrimitiveValue> CreateFontFamilyValue(const CSSOMStringAtom &familyName) noexcept;
  };

  inline PrimitiveValue &PrimitiveValue::ImplicitInitialValue() noexcept
  {
    return CommonValuePool->_implicitInitialValue;
  }

  inline Ref<PrimitiveValue> PrimitiveValue::Create(css::ValueId identifier) noexcept
  {
    krys_assert(ToUnderlying(identifier) < TotalValueKeywords);
    return ShareRef(*CommonValuePool->_identifierValues[ToUnderlying(identifier)]);
  }
}
