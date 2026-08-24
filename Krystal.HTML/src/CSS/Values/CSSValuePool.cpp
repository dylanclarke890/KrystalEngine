#include "Krystal.HTML/CSS/Values/CSSValuePool.hpp"

namespace Krys::HTML
{
  LazyNeverDestroyed<StaticCSSValuePool> CommonCSSValuePool;

  StaticCSSValuePool::StaticCSSValuePool() noexcept
      : _implicitInitialValue(CSSValue::StaticCSSValue, CSSPrimitiveValue::CreateImplicitInitialValue),
        _transparentColor(CSSValue::StaticCSSValue, WebCore::Color::transparentBlack),
        _whiteColor(CSSValue::StaticCSSValue, WebCore::Color::white),
        _blackColor(CSSValue::StaticCSSValue, WebCore::Color::black)
  {
    for (auto keyword : AllCSSValueKeywords())
    {
      new (_identifierValues[ToUnderlying(keyword)].get())
        CSSPrimitiveValue {CSSValue::StaticCSSValue, keyword};
    }

    for (size_t i = 0uz; i <= MaximumCacheableIntegerValue; ++i)
    {
      new (_pixelValues[i].get()) CSSPrimitiveValue(CSSValue::StaticCSSValue, i, CSSUnitType::px);
      new (_percentageValues[i].get())
        CSSPrimitiveValue(CSSValue::StaticCSSValue, i, CSSUnitType::Percentage);
      new (_numberValues[i].get()) CSSPrimitiveValue(CSSValue::StaticCSSValue, i, CSSUnitType::Number);
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