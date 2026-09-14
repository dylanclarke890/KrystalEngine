#pragma once

#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Utils/AlignedStorage.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::css
{
  class ValueList;
  class ValuePool;

  class StaticValuePool
  {
    friend class PrimitiveValue;
    friend class ValuePool;
    friend class LazyNeverDestroyed<StaticValuePool>;

  private:
    PrimitiveValue _implicitInitialValue;

    ColorValue _transparentColor;
    ColorValue _whiteColor;
    ColorValue _blackColor;

    constexpr static size_t MaximumCacheableIntegerValue = 255uz;
    Array<AlignedStorage<PrimitiveValue>, MaximumCacheableIntegerValue + 1uz> _pixelValues;
    Array<AlignedStorage<PrimitiveValue>, MaximumCacheableIntegerValue + 1uz> _percentageValues;
    Array<AlignedStorage<PrimitiveValue>, MaximumCacheableIntegerValue + 1uz> _numberValues;
    Array<AlignedStorage<PrimitiveValue>, TotalValueKeywords> _identifierValues;

    StaticValuePool() noexcept;

  public:
    static void Init() noexcept;
  };

  extern LazyNeverDestroyed<StaticValuePool> CommonValuePool;

  inline PrimitiveValue &PrimitiveValue::ImplicitInitialValue() noexcept
  {
    return CommonValuePool->_implicitInitialValue;
  }

  inline Ref<PrimitiveValue> PrimitiveValue::Create(css::ValueId identifier) noexcept
  {
    krys_debug_assert(static_cast<underlying_t<css::ValueId>>(identifier) < TotalValueKeywords);

    auto &value = *CommonValuePool->_identifierValues[static_cast<underlying_t<css::ValueId>>(identifier)];
    return ShareRef(value);
  }

  class ValuePool
  {
  public:
    KRYS_NODISCARD static ValuePool &MainThreadPool() noexcept;
  };
}