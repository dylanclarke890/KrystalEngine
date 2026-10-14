#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParser.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Core/Maths/Random.hpp"
#include <iterator>
#include <mutex>

namespace krys::boo::css
{
  namespace
  {
    // Remove one entry at random if the cache grows too large.
    // FIXME: Use TinyLRUCache instead?
    template <typename TMap>
    void RemoveRandomEntryIfFull(TMap &map, size_t maximumSize) noexcept
    {
      if (map.size() >= maximumSize)
      {
        auto index = Random::UInt(static_cast<uint32>(map.size() - 1uz));
        map.erase(std::next(map.begin(), static_cast<ptrdiff_t>(index)));
      }
    }
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
      auto value = static_cast<double>(i);
      new (_pixelValues[i].get()) PrimitiveValue(Value::StaticValue, value, UnitType::px);
      new (_percentageValues[i].get()) PrimitiveValue(Value::StaticValue, value, UnitType::Percentage);
      new (_numberValues[i].get()) PrimitiveValue(Value::StaticValue, value, UnitType::Number);
    }
  }

  void StaticValuePool::Init() noexcept
  {
    static std::once_flag onceKey;
    std::call_once(onceKey, []() { CommonValuePool.Construct(); });
  }

  ValuePool &ValuePool::MainThreadPool() noexcept
  {
    static MainThreadNeverDestroyed<ValuePool> pool;
    return pool;
  }

  // Defined here, where `ValueList` is complete.
  ValuePool::ValuePool() noexcept = default;
  ValuePool::~ValuePool() noexcept = default;

  Ref<ColorValue> ValuePool::CreateColorValue(const boo::Color &color) noexcept
  {
    // These are the empty and deleted values of the hash table.
    if (color == boo::Color::transparentBlack)
    {
      return ShareRef(CommonValuePool->_transparentColor);
    }
    if (color == boo::Color::white)
    {
      return ShareRef(CommonValuePool->_whiteColor);
    }
    // Just because it is common.
    if (color == boo::Color::black)
    {
      return ShareRef(CommonValuePool->_blackColor);
    }

    constexpr size_t maximumColorCacheSize = 512uz;
    RemoveRandomEntryIfFull(_colorValueCache, maximumColorCacheSize);

    if (auto it = _colorValueCache.find(color); it != _colorValueCache.end())
    {
      return it->second;
    }
    return _colorValueCache.emplace(color, ColorValue::Create(color)).first->second;
  }

  RefPtr<ValueList> ValuePool::CreateFontFaceValue(const CSSOMStringAtom &string) noexcept
  {
    constexpr size_t maximumFontFaceCacheSize = 128uz;
    RemoveRandomEntryIfFull(_fontFaceValueCache, maximumFontFaceCacheSize);

    if (auto it = _fontFaceValueCache.find(string); it != _fontFaceValueCache.end())
    {
      return it->second;
    }

    auto value = PropertyParser::ParseStylePropertyLonghand(PropertyId::FontFamily, CSSOMString {string.View()},
                                                            StrictParserContext());
    auto *list = DynamicDowncast<ValueList>(value.get());
    return _fontFaceValueCache.emplace(string, ShareRefPtr(list)).first->second;
  }

  Ref<PrimitiveValue> ValuePool::CreateFontFamilyValue(const CSSOMStringAtom &familyName) noexcept
  {
    constexpr size_t maximumFontFamilyCacheSize = 128uz;
    RemoveRandomEntryIfFull(_fontFamilyValueCache, maximumFontFamilyCacheSize);

    if (auto it = _fontFamilyValueCache.find(familyName); it != _fontFamilyValueCache.end())
    {
      return it->second;
    }
    auto value = PrimitiveValue::CreateFontFamily(CSSOMString {familyName.View()});
    return _fontFamilyValueCache.emplace(familyName, krys::move(value)).first->second;
  }

  void ValuePool::Drain() noexcept
  {
    _colorValueCache.clear();
    _fontFaceValueCache.clear();
    _fontFamilyValueCache.clear();
  }
}
