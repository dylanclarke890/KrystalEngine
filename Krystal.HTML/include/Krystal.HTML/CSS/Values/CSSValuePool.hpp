#pragma once

#include "Krystal.HTML/CSS/Values/CSSColorValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Lib/AlignedStorage.hpp"
#include "Krystal.Lib/NeverDestroyed.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CSSValueList;
  class CSSValuePool;

  class StaticCSSValuePool
  {
    friend class CSSPrimitiveValue;
    friend class CSSValuePool;
    friend class LazyNeverDestroyed<StaticCSSValuePool>;

  private:
    CSSPrimitiveValue _implicitInitialValue;

    CSSColorValue _transparentColor;
    CSSColorValue _whiteColor;
    CSSColorValue _blackColor;

    constexpr static size_t MaximumCacheableIntegerValue = 255uz;
    Array<AlignedStorage<CSSPrimitiveValue>, MaximumCacheableIntegerValue + 1uz> _pixelValues;
    Array<AlignedStorage<CSSPrimitiveValue>, MaximumCacheableIntegerValue + 1uz> _percentageValues;
    Array<AlignedStorage<CSSPrimitiveValue>, MaximumCacheableIntegerValue + 1uz> _numberValues;
    Array<AlignedStorage<CSSPrimitiveValue>, NumCSSValueKeywords> _identifierValues;

    StaticCSSValuePool() noexcept;

  public:
    static void Init() noexcept;
  };

  extern LazyNeverDestroyed<StaticCSSValuePool> CommonCSSValuePool;

  inline CSSPrimitiveValue &CSSPrimitiveValue::ImplicitInitialValue() noexcept
  {
    return CommonCSSValuePool->_implicitInitialValue;
  }

  inline Ref<CSSPrimitiveValue> CSSPrimitiveValue::Create(CSSValueId identifier) noexcept
  {
    assert(static_cast<underlying_t<CSSValueId>>(identifier) < NumCSSValueKeywords);

    auto &value = *CommonCSSValuePool->_identifierValues[static_cast<underlying_t<CSSValueId>>(identifier)];
    return ShareRef(value);
  }

  class CSSValuePool
  {
  public:
    KRYS_NODISCARD static CSSValuePool &MainThreadPool() noexcept;
  };
}