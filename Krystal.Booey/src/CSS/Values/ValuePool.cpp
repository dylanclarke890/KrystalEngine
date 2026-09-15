#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"

namespace krys::boo::css
{
  ValuePool &ValuePool::MainThreadPool() noexcept
  {
    static MainThreadNeverDestroyed<ValuePool> pool;
    return pool;
  }

  LazyNeverDestroyed<StaticValuePool> CommonValuePool;

  StaticValuePool::StaticValuePool() noexcept
      : _implicitInitialValue(Value::StaticValue, PrimitiveValue::CreateImplicitInitialValue),
        _transparentColor(Value::StaticValue, boo::Color::transparentBlack),
        _whiteColor(Value::StaticValue, boo::Color::white), _blackColor(Value::StaticValue, boo::Color::black)
  {
    for (auto keyword : AllValueKeywords())
    {
      new (_identifierValues[ToUnderlying(keyword)].get()) PrimitiveValue {Value::StaticValue, keyword};
    }

    for (size_t i = 0uz; i <= MaximumCacheableIntegerValue; ++i)
    {
      double v = static_cast<double>(i);

      new (_pixelValues[i].get()) PrimitiveValue(Value::StaticValue, v, UnitType::px);
      new (_percentageValues[i].get()) PrimitiveValue(Value::StaticValue, v, UnitType::Percentage);
      new (_numberValues[i].get()) PrimitiveValue(Value::StaticValue, v, UnitType::Number);
    }
  }

  void StaticValuePool::Init() noexcept
  {
    static std::once_flag onceKey;

    std::call_once(onceKey, []() { CommonValuePool.Construct(); });
  }
}