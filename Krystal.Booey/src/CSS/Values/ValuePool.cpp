#include "Krystal.Booey/CSS/Values/CSSValuePool.hpp"
#include "Krystal.Core/Color/Color.hpp"

namespace krys::boo::css
{
  LazyNeverDestroyed<StaticCSSValuePool> CommonCSSValuePool;

  StaticCSSValuePool::StaticCSSValuePool() noexcept
      : _implicitInitialValue(CSSValue::StaticCSSValue, CSSPrimitiveValue::CreateImplicitInitialValue),
        _transparentColor(CSSValue::StaticCSSValue, krys::Color::transparentBlack),
        _whiteColor(CSSValue::StaticCSSValue, krys::Color::white),
        _blackColor(CSSValue::StaticCSSValue, krys::Color::black)
  {
    for (auto keyword : AllValueKeywords())
    {
      new (_identifierValues[ToUnderlying(keyword)].get())
        CSSPrimitiveValue {CSSValue::StaticCSSValue, keyword};
    }

    for (double i = 0; i <= MaximumCacheableIntegerValue; ++i)
    {
      new (_pixelValues[static_cast<size_t>(i)].get())
        CSSPrimitiveValue(CSSValue::StaticCSSValue, i, UnitType::px);

      new (_percentageValues[static_cast<size_t>(i)].get())
        CSSPrimitiveValue(CSSValue::StaticCSSValue, i, UnitType::Percentage);

      new (_numberValues[static_cast<size_t>(i)].get())
        CSSPrimitiveValue(CSSValue::StaticCSSValue, i, UnitType::Number);
    }
  }

  void StaticCSSValuePool::Init() noexcept
  {
    static std::once_flag onceKey;
    std::call_once(onceKey, []() { CommonCSSValuePool.Construct(); });
  }

  CSSValuePool &CSSValuePool::MainThreadPool() noexcept
  {
    static MainThreadNeverDestroyed<CSSValuePool> pool;
    return pool;
  }
}