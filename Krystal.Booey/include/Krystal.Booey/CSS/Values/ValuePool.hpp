#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Utils/AlignedStorage.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

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
    HashMap<CSSOMStringAtom, Ref<PrimitiveValue>> _fontFamilyValueCache;

  public:
    ValuePool() noexcept = default;

    // FIXME: This function needs a name that make it clear that this is not the one and only CSSValuePool,
    // rather the one that can be used only on the main thread.
    // FIXME: Consider a design where the value pool thread-local storage so callers don't have to deal
    // directly with the value pool at all.
    KRYS_NODISCARD static ValuePool &MainThreadPool() noexcept;

    void Drain() noexcept;

    KRYS_NODISCARD Ref<ColorValue> CreateColorValue(const boo::Color &color) noexcept;
    // TODO: CreateFontFaceValue(const CSSOMStringAtom &) -> RefPtr<ValueList>, once ValueList and
    // PropertyParser::ParseStylePropertyLonghand are ported.
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
